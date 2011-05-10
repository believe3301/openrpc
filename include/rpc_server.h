/*
 * rpc_server.h
 *
 *  Created on: 2011-3-20
 *      Author: yanghu
 */

#ifndef RPC_SERVER_H_
#define RPC_SERVER_H_

#include "rpc_common.h"
#include "rpc_hash.h"
#include <pthread.h>

BEGIN_DECLS

struct _rpc_server {
	int port;
	int init_count;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	rpc_dispatch_thread *th_dispatch;
	rpc_worker_thread *th_workers;
	int worker_len;
	rpc_hash_table *service_map;
};

rpc_server* rpc_server_create(int port);

void rpc_server_regservice(rpc_server *server, char *service_name,
		char *method_name, rpc_function cb);

boolean rpc_server_start(rpc_server *server);

void rpc_server_stop(rpc_server *server);

END_DECLS
#endif /* RPC_SERVER_H_ */
