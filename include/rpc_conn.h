/*
 * rpc_conn.h
 *
 *  Created on: 2011-3-20
 *      Author: yanghu
 */

#ifndef RPC_CONN_H_
#define RPC_CONN_H_

#include "rpc_common.h"
#include <ev.h>

struct _rpc_conn {
	int sfd;
	int curr_seq;
	struct ev_loop *loop;
	struct ev_io watcher;

	char *rbuf;
	char *rcurr;
	int rsize;
	int rbytes;

	struct iovec *iov;
	int iovsize;
	int iovused;

	void *unprocess_data;//unprocess data
	void *thread;//belong to thread
};

rpc_conn *rpc_conn_new(int fd, struct ev_loop* l);

rpc_conn *rpc_conn_client_new(int fd, struct ev_loop *l);

void rpc_conn_addiov(rpc_conn *c, pointer data, size_t data_len);

void rpc_send_message(rpc_conn *conn);

void rpc_conn_close(rpc_conn *c);

void rpc_return(rpc_conn *conn, pointer output, size_t output_len);

void rpc_return_null(rpc_conn *conn);

void rpc_return_error(rpc_conn *conn, rpc_code err_code, char* err_message);
#endif /* RPC_CONN_H_ */
