/*
 * rpc_thread.h
 *
 *  Created on: 2011-3-21
 *      Author: yanghu
 */

#ifndef RPC_THREAD_H_
#define RPC_THREAD_H_

#include "rpc_common.h"
#include "rpc_array.h"
#include "rpc_queue.h"
#include "rpc_async_queue.h"
#include "rpc_sessionpool.h"
#include <pthread.h>
#include <ev.h>

#define CLIENT_CONN_NUM 3

struct _rpc_dispatch_thread {
	pthread_t thread_id;
	int sock_fd;
	int last_thread;
	struct ev_loop *loop;
	struct ev_io watcher;
	rpc_server *server;
};

struct _rpc_worker_thread {
	pthread_t thread_id;
	int notify_fd;
	struct ev_loop *loop;
	struct ev_io watcher;
	rpc_queue *queue;
	rpc_server *server;
};

//ev_io only init one fd?
struct _rpc_client_thread {
	pthread_t thread_send_id;
	pthread_t thread_receive_id;

	struct ev_loop *loop;

	rpc_async_queue *req_pending;//request pending queue
	rpc_sessionpool *req_pool;//request session pool
	rpc_sessionpool *req_timer;//request timer pool

	int req_conn_count;
	int last_conn;
	rpc_array *req_conns;//request connections

	rpc_client *client;
};

boolean rpc_dispatch_init(rpc_dispatch_thread *th);

boolean rpc_dispatch_start(rpc_dispatch_thread *th);

void rpc_worker_init(rpc_worker_thread *th);

void rpc_worker_start(rpc_worker_thread *th);

boolean rpc_client_thread_init(rpc_client *client, rpc_client_thread *th);

boolean rpc_client_thread_start(rpc_client_thread *th);

#endif /* RPC_THREAD_H_ */
