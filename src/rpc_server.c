/*
 * rpc_server.c
 *
 *  Created on: 2011-3-20
 *      Author: yanghu
 */

#include "rpc_server.h"
#include "rpc_thread.h"
#include "rpc_util.h"
#include "rpc_hash.h"
#include <ev.h>
#include <assert.h>

rpc_server* rpc_server_create(int port) {
	rpc_server *srv = rpc_new(rpc_server,1);
	srv->service_map = rpc_hash_table_new();
	srv->worker_len = WORKER_LEN;
	srv->th_dispatch = rpc_new(rpc_dispatch_thread,1);
	srv->th_workers = rpc_new(rpc_worker_thread,srv->worker_len);
	srv->port = port;
	pthread_cond_init(&srv->cond, NULL);
	pthread_mutex_init(&srv->mutex, NULL);
	srv->init_count = 0;
	return srv;
}

void rpc_server_regservice(rpc_server *server, char *service_name,
		char *method_name, rpc_function cb) {
	assert(server!=NULL);
	assert(service_name !=NULL);
	assert(method_name !=NULL);

	rpc_service *service = rpc_new(rpc_service,1);
	service->service_name = service_name;
	service->method_name = method_name;
	service->cb = cb;

	char *key = rpc_vsprintf("%s@%s", method_name, service_name);
	rpc_hash_table_insert(server->service_map, key, service);
}

boolean rpc_server_start(rpc_server *server) {
	int i;
	for (i = 0; i < server->worker_len; ++i) {
		rpc_worker_init(&server->th_workers[i]);
		server->th_workers[i].server = server;
	}
	for (i = 0; i < server->worker_len; ++i) {
		rpc_worker_start(&server->th_workers[i]);
	}
	pthread_mutex_lock(&server->mutex);
	while (server->init_count < server->worker_len) {
		pthread_cond_wait(&server->cond, &server->mutex);
	}
	pthread_mutex_unlock(&server->mutex);

	//dispatch thread
	server->th_dispatch->server = server;
	if (rpc_dispatch_init(server->th_dispatch)) {
		rpc_dispatch_start(server->th_dispatch);
		return TRUE;
	}
	return FALSE;
}
