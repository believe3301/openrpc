/*
 * rpc_thread.c
 *
 *  Created on: 2011-3-21
 *      Author: yanghu
 */
#include "rpc_thread.h"
#include "rpc_server.h"
#include "rpc_client.h"
#include "rpc_conn.h"
#include "rpc_request.h"
#include "rpc_util.h"

#include <assert.h>
#include <memory.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/eventfd.h>

//TODO
//get thread from free thread list

static void cb_notify_conn(struct ev_loop *l, struct ev_io *watcher,
		int revents) {
	rpc_worker_thread *worker = watcher->data;
	uint64_t n;
	read(worker->notify_fd, &n, sizeof(n));
	if (n != 1) {
		perror("read notify data error");
	}
	//new conn;
	int cfd = POINTER_TO_INT(rpc_queue_pop(worker->queue));
	rpc_conn *c = rpc_conn_new(cfd, worker->loop);
	c->thread = worker;
}

static void cb_dispatch_conn(struct ev_loop *l, struct ev_io *watcher,
		int revents) {
	rpc_dispatch_thread *th = watcher->data;
	int cfd;
	if ((cfd = accept(th->sock_fd, NULL, NULL)) == -1) {
		//check errno
		perror("accept conn error");
		return;
	}
	rpc_set_non_block(cfd);
	//rrd thread
	int idx;
	idx = (th->last_thread + 1) % (th->server->worker_len);
	th->last_thread = idx;
	rpc_worker_thread *worker = th->server->th_workers + idx;
	//notify
	rpc_queue_push(worker->queue, INT_TO_POINTER(cfd));
	uint64_t n = 1;
	write(worker->notify_fd, &n, sizeof(n));
}

void rpc_worker_init(rpc_worker_thread *th) {
	th->loop = ev_loop_new(0);
	th->notify_fd = eventfd(0, 0);
	th->queue = rpc_queue_new();
	th->watcher.data = th;
	ev_io_init(&th->watcher,cb_notify_conn,th->notify_fd,EV_READ);
	ev_io_start(th->loop, &th->watcher);
}

static void* thread_worker_handler(void* data) {
	rpc_worker_thread *th = data;
	pthread_mutex_lock(&(th->server->mutex));
	th->server->init_count++;
	pthread_cond_signal(&(th->server->cond));
	pthread_mutex_unlock(&(th->server->mutex));

	th->thread_id = pthread_self();
	ev_run(th->loop, 0);
	return NULL;
}

static void* thread_dispatch_handler(void* data) {
	rpc_dispatch_thread *th = data;
	th->thread_id = pthread_self();
	ev_run(th->loop, 0);
	return NULL;
}

void rpc_worker_start(rpc_worker_thread *th) {
	pthread_t pid;
	pthread_create(&pid, NULL, thread_worker_handler, th);
}

boolean rpc_dispatch_init(rpc_dispatch_thread *th) {
	assert(th!=NULL);

	int port = th->server->port;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	int sfd = socket(AF_INET, SOCK_STREAM, 0);

	int flag = 1;
	//keepalive
	setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag));
	//reuse
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	if (bind(sfd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		perror("bind");
		return FALSE;
	}

	if (listen(sfd, BACKLOG) == -1) {
		perror("accept");
		return FALSE;
	}
	rpc_set_non_block(sfd);

	th->sock_fd = sfd;

	th->loop = ev_loop_new(0);
	th->last_thread = -1;
	th->watcher.data = th;
	ev_io_init(&th->watcher,cb_dispatch_conn,sfd,EV_READ);
	ev_io_start(th->loop, &th->watcher);
	return TRUE;
}

boolean rpc_dispatch_start(rpc_dispatch_thread *th) {
	pthread_t pid;
	int ret = pthread_create(&pid, NULL, thread_dispatch_handler, th);
	if (ret == -1) {
		perror("create dispatch thread error");
		return FALSE;
	}
	return TRUE;
}

boolean rpc_client_thread_init(rpc_client *client, rpc_client_thread *th) {
	assert(th!=NULL);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(client->port);
	inet_aton(client->host, &addr.sin_addr);

	th->loop = ev_loop_new(0);
	th->req_conn_count = CLIENT_CONN_NUM;
	th->req_conns = rpc_array_new();
	th->last_conn = -1;
	int i, cfd;
	rpc_conn *c = NULL;
	for (i = 0; i < th->req_conn_count; i++) {
		cfd = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(cfd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
			perror("connect");
			return FALSE;
		}
		rpc_set_non_block(cfd);
		c = rpc_conn_client_new(cfd, th->loop);
		c->thread = th;
		rpc_array_add(th->req_conns, c);
	}
	th->req_pending = rpc_async_queue_new();
	th->req_pool = rpc_sessionpool_new();
	th->req_timer = rpc_sessionpool_new();
	th->client = client;
	c->thread = th;
	return TRUE;
}

static void* thread_write_hander(void* data) {
	rpc_client_thread *th = data;
	//send pending request
	rpc_request *req = NULL;
	rpc_conn *conn = NULL;

	int idx;
	for (;;) {
		req = (rpc_request*) rpc_async_queue_pop(th->req_pending);
		//push pool
		req->seq = rpc_sessionpool_insert(th->req_pool, req);

		idx = (th->last_conn + 1) % (th->req_conn_count);
		conn = (rpc_conn*) rpc_array_index(th->req_conns, idx);
		th->last_conn = idx;

		//write data
		rpc_request_format(req, conn);
		rpc_send_message(conn);
	}
	return NULL;
}

static void* thread_client_handler(void* data) {
	rpc_client_thread *th = data;
	pthread_mutex_t mutex = th->client->mutex;
	pthread_mutex_lock(&mutex);
	th->client->init_count++;
	pthread_cond_signal(&(th->client->cond));
	pthread_mutex_unlock(&mutex);
	th->thread_receive_id = pthread_self();
	ev_run(th->loop, 0);
	return NULL;
}

boolean rpc_client_thread_start(rpc_client_thread *th) {
	assert(th!=NULL);
	//read
	pthread_t pid;
	pthread_create(&pid, NULL, thread_client_handler, th);
	//write
	pthread_create(&pid, NULL, thread_write_hander, th);
	return TRUE;
}
