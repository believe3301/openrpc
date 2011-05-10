/*
 * rpc_conn.c
 *
 *  Created on: 2011-3-20
 *      Author: yanghu
 */

#include "rpc_conn.h"
#include "rpc_request.h"
#include "rpc_response.h"
#include "rpc_thread.h"
#include "rpc_server.h"
#include "rpc_util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

static rpc_conn **freeconns;
static int freetotal = 200;
static int freecurr = 0;
static pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER;

static void rpc_freelist_init() {
	if (freeconns == NULL) {
		pthread_mutex_lock(&conn_lock);
		if (freeconns == NULL) {
			freeconns = rpc_new0(rpc_conn*,freetotal);
		}
		pthread_mutex_unlock(&conn_lock);
	}
}

rpc_conn* rpc_conn_from_freelist() {
	rpc_freelist_init();
	rpc_conn *conn = NULL;
	pthread_mutex_lock(&conn_lock);
	if (freecurr > 0) {
		conn = freeconns[freecurr--];
	}
	pthread_mutex_unlock(&conn_lock);
	return conn;
}

void rpc_conn_add_freelist(rpc_conn *c) {
	rpc_freelist_init();
	pthread_mutex_lock(&conn_lock);
	if (freecurr < freetotal) {
		freeconns[freecurr++] = c;
	} else {
		size_t newsize = freetotal * 2;
		rpc_conn **new_freeconns = rpc_realloc(freeconns, sizeof(rpc_conn *)
				* newsize);
		if (new_freeconns) {
			freetotal = newsize;
			freeconns = new_freeconns;
			freeconns[freecurr++] = c;
		}
	}
	pthread_mutex_unlock(&conn_lock);
}

static boolean read_from_conn(rpc_conn *c) {
	if (c->rcurr != c->rbuf) {
		if (c->rbytes != 0)
			memcpy(c->rbuf, c->rcurr, c->rbytes);
		c->rcurr = c->rbuf;
	}
	int num_allocs = 0;
	for (;;) {
		if (c->rbytes >= c->rsize) {
			if (num_allocs == 4) {
				return TRUE;
			}
			++num_allocs;
			char *new_rbuf = rpc_realloc(c->rbuf, c->rsize * 2);
			if (!new_rbuf) {
				break;
			}
			c->rcurr = c->rbuf = new_rbuf;
			c->rsize *= 2;
		}
		int avail = c->rsize - c->rbytes;
		int n = read(c->sfd, c->rbuf + c->rbytes, avail);
		if (n == 0) {
			fprintf(stderr, "connected closed!\n");
			rpc_conn_close(c);
			return FALSE;
		} else if (n == -1) {
			if (errno == EINTR || errno == EWOULDBLOCK) {
				continue;
			}
			perror("read error!");
			rpc_conn_close(c);
			break;
		} else {
			c->rbytes += n;
			if (n == avail) {
				continue;
			} else {
				break;
			}
		}
	}
	return TRUE;
}

//check has received head
static boolean rpc_conn_received_head(const pointer data, const size_t len) {
	char *el, *bl, *cont;
	el = NULL;
	bl = NULL;
	int avail;
	avail = len;
	cont = data;
	while ((el = memchr(cont, '\n', avail)) != NULL) {
		if (bl) {
			if ((el - bl == 1) || ((el - bl) == 2 && *(el - 1) == '\r')) {
				return TRUE;
			}
		}
		avail -= (el + 1 - cont);
		if (avail < 0) {
			return FALSE;
		} else if (avail == 0) {
			if (*(el + 1) == '\n')
				return TRUE;
		}
		bl = memchr(el + 1, '\n', avail);
		if (bl) {
			if ((bl - el == 1) || ((bl - el) == 2 && *(bl - 1) == '\r')) {
				return TRUE;
			} else {
				avail -= (bl + 1 - el);
				if (avail <= 0)
					return FALSE;
				else
					cont = bl + 1;
			}
		} else {
			return FALSE;
		}
	}
	return FALSE;
}

static void cb_req_read(struct ev_loop *l, struct ev_io *watcher, int revents) {
	rpc_conn *c = watcher->data;
	//TODO
	//one conn would starve thread
	if (read_from_conn(c)) {
		for (;;) {
			if (c->rbytes <= 0) {
				return;
			}
			rpc_request *req = NULL;
			rpc_parse_result result;
			boolean has_copyhead = FALSE;
			if (c->unprocess_data == NULL) {
				if (rpc_conn_received_head(c->rcurr, c->rbytes)) {
					result = rpc_request_parse(c, &req);
					if (result == RPC_Parse_Error) {
						fprintf(stderr, "request parse error!\n");
						return;
					} else if (result == RPC_Parse_NeedData) {
						c->unprocess_data = rpc_request_copy_head(req);
						rpc_request_free(req);
						return;
					}
				} else {
					return;
				}
			} else {
				rpc_request *last_request;
				last_request = (rpc_request*) c->unprocess_data;
				if (c->rbytes >= last_request->input_len) {
					last_request->input = c->rcurr;
					c->rcurr += last_request->input_len;
					c->rbytes -= last_request->input_len;
					req = last_request;
					c->unprocess_data = NULL;
					has_copyhead = TRUE;
				} else {
					return;
				}
			}
			rpc_worker_thread *th = (rpc_worker_thread*) c->thread;
			rpc_server *server = th->server;
			char key[256];
			int len = snprintf(key, 255, "%s@%s", req->method_name,
					req->service_name);
			key[len] = '\0';
			rpc_service *service =
					rpc_hash_table_lookup(server->service_map, key);
			if (service) {
				c->curr_seq = req->seq;
				service->cb(c, req->input, req->input_len, service->data);
			} else {
				rpc_return_error(c, RPC_NotFound, "not find method@service");
			}
			if (has_copyhead) {
				rpc_free(req->method_name);
				rpc_free(req->service_name);
				rpc_free(req->rpc_version);
			}
			rpc_request_free(req);
		}
	} else {
		fprintf(stderr, "read request error!\n");
	}
}

static void cb_rsp_read(struct ev_loop *l, struct ev_io *watcher, int revents) {
	rpc_conn *c = watcher->data;
	//read response
	if (read_from_conn(c)) {
		for (;;) {
			if (c->rbytes <= 0) {
				return;
			}
			boolean has_copyhead = FALSE;
			rpc_response *rsp;
			rpc_parse_result result;
			if (c->unprocess_data == NULL) {
				if (rpc_conn_received_head(c->rcurr, c->rbytes)) {
					result = rpc_response_parse(c, &rsp);
					if (result == RPC_Parse_Error) {
						fprintf(stderr, "response parse error!\n");
						return;
					} else if (result == RPC_Parse_NeedData) {
						c->unprocess_data = rpc_response_copy_head(rsp);
						rpc_response_free(rsp);
						return;
					}
				} else {
					return;
				}
			} else {
				rpc_response *last_response;
				last_response = (rpc_response*) c->unprocess_data;
				if (c->rbytes >= last_response->output_len) {
					last_response->output = c->rcurr;
					c->rcurr += last_response->output_len;
					c->rbytes -= last_response->output_len;
					rsp = last_response;
					c->unprocess_data = NULL;
					has_copyhead = TRUE;
				} else {
					return;
				}
			}
			rpc_client_thread *th = (rpc_client_thread *) c->thread;
			//pop send queue

			rpc_request *req = NULL;
			//TODO why null
			//sessionpool has
			req = (rpc_request*) rpc_sessionpool_get(th->req_pool, rsp->seq);
			if (req->seq != rsp->seq) {
				fprintf(stderr, "seq not equal!\n");
				return;
			}
			//TODO
			//callback using threadpool
			req->callback(c, rsp->code, rsp->output, rsp->output_len, req->data);
			rpc_sessionpool_remove(th->req_pool, rsp->seq);
			rpc_request_free(req);
			if (has_copyhead) {
				rpc_free(rsp->phrase);
			}
			rpc_response_free(rsp);
		}
	} else {
		fprintf(stderr, "read response error!\n");
	}
}

void rpc_conn_close(rpc_conn *c) {
	assert(c!=NULL);
	ev_io_stop(c->loop, &c->watcher);
	close(c->sfd);
	c->rcurr = 0;
	c->iovused = 0;
	c->thread = NULL;
	c->loop = NULL;
	rpc_conn_add_freelist(c);
}

static rpc_conn *rpc_conn_new_inner(int fd, struct ev_loop* l, void(*cb)(
		struct ev_loop *l, struct ev_io *watcher, int revents)) {
	rpc_conn *conn;
	conn = rpc_conn_from_freelist();
	if (conn == NULL) {
		conn = rpc_new0(rpc_conn,1);
		conn->rbuf = rpc_new(char,BUFFER_SIZE);
		conn->rsize = BUFFER_SIZE;

		conn->iovsize = 5;
		conn->iov = rpc_new(struct iovec,conn->iovsize);
	}
	conn->rbytes = 0;
	conn->rcurr = conn->rbuf;
	conn->iovused = 0;
	conn->curr_seq = -1;
	conn->unprocess_data = NULL;
	conn->watcher.data = conn;
	conn->sfd = fd;
	conn->loop = l;
	ev_io_init(&conn->watcher,cb,fd,EV_READ);
	ev_io_start(l, &conn->watcher);
	return conn;
}

rpc_conn *rpc_conn_new(int fd, struct ev_loop* l) {
	return rpc_conn_new_inner(fd, l, cb_req_read);
}

rpc_conn *rpc_conn_client_new(int fd, struct ev_loop* l) {
	return rpc_conn_new_inner(fd, l, cb_rsp_read);
}

void rpc_conn_update_event(rpc_conn *c, const int new_flags) {
	//ev_io_set not active?
	ev_io_stop(c->loop, &c->watcher);
	ev_io_set(&c->watcher,c->sfd,new_flags);
	ev_io_start(c->loop, &c->watcher);
}

void rpc_conn_addiov(rpc_conn *c, pointer data, size_t data_len) {
	if (c->iovused >= c->iovsize) {
		int new_size = (c->iovsize) * 2;
		pointer new_iov = rpc_realloc(c->iov, sizeof(struct iovec) * new_size);
		if (new_iov) {
			c->iov = (struct iovec*) new_iov;
			c->iovsize = new_size;
		}
	}
	c->iov[c->iovused].iov_base = data;
	c->iov[c->iovused].iov_len = data_len;
	c->iovused++;
}

void rpc_send_message(rpc_conn *conn) {
	struct msghdr m;
	memset(&m, 0, sizeof(m));
	m.msg_iov = conn->iov;
	m.msg_iovlen = conn->iovused;
	ssize_t res;
	for (;;) {
		res = sendmsg(conn->sfd, &m, 0);
		if (res > 0) {
			while (m.msg_iovlen > 0 && res >= m.msg_iov->iov_len) {
				res -= m.msg_iov->iov_len;
				m.msg_iovlen--;
				m.msg_iov++;
			}

			if (res > 0) {
				m.msg_iov->iov_base = (caddr_t) m.msg_iov->iov_base + res;
				m.msg_iov->iov_len -= res;
			} else {
				break;
			}
		} else if (res == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			} else {
				perror("failed to write");
				rpc_conn_close(conn);
			}
		} else {
			break;
		}
	}
	//TODO free resource
	conn->iovused = 0;
}

static inline void rpc_send_response(rpc_conn *conn, rpc_code code,
		pointer output, size_t output_len) {
	//status line
	char *data;
	data = rpc_vsprintf("%s %d %s\n", RPC_VERSION, (int) code, rpc_code_format(
			code));
	rpc_conn_addiov(conn, data, strlen(data));
	//seq
	data = rpc_vsprintf("seq:%d\n", conn->curr_seq);
	rpc_conn_addiov(conn, data, strlen(data));
	//body-len
	data = rpc_vsprintf("body-len:%d\n\n", output_len);
	rpc_conn_addiov(conn, data, strlen(data));
	//body
	if (output) {
		rpc_conn_addiov(conn, output, output_len);
	}
	rpc_send_message(conn);
}

void rpc_return(rpc_conn *conn, pointer output, size_t output_len) {
	rpc_send_response(conn, RPC_OK, output, output_len);
}

void rpc_return_null(rpc_conn *conn) {
	rpc_send_response(conn, RPC_OK, NULL, 0);
}

void rpc_return_error(rpc_conn *conn, rpc_code err_code, char* err_message) {
	rpc_send_response(conn, err_code, err_message, strlen(err_message));
}

void rpc_return_error_fmt(rpc_conn *conn, rpc_code err_code,
		char* err_message, ...) {
}
