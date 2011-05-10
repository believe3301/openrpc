/*
 * rpc_client.c
 *
 *  Created on: 2011-3-21
 *      Author: yanghu
 */

#include "rpc_client.h"
#include "rpc_request.h"
#include "rpc_response.h"
#include "rpc_thread.h"
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <time.h>
#include <sys/eventfd.h>

//server: one thread multiply connection. a connection would block a thread
//client: one thread one connection

//three thread,three conn,send queue/pending queue
//if connection disconnected?

rpc_client* rpc_client_new() {
	rpc_client *client = rpc_new0(rpc_client,1);
	client->host = NULL;
	client->port = 0;
	client->thread_count = CLIENT_THREAD_NUM;
	client->threads = rpc_new(rpc_client_thread,CLIENT_THREAD_NUM);
	client->last_thread = -1;
	client->init_count = 0;
	pthread_mutex_init(&client->mutex, NULL);
	pthread_cond_init(&client->cond, NULL);
	return client;
}

boolean rpc_client_connect(rpc_client* client, char* host, int port) {
	assert(client!=NULL);
	client->host = host;
	client->port = port;
	int i;
	for (i = 0; i < client->thread_count; ++i) {
		if (!rpc_client_thread_init(client, &client->threads[i]))
			return FALSE;
	}
	for (i = 0; i < client->thread_count; ++i) {
		if (!rpc_client_thread_start(&client->threads[i])) {
			return FALSE;
		}
	}
	pthread_mutex_lock(&client->mutex);
	while (client->init_count < client->thread_count) {
		pthread_cond_wait(&client->cond, &client->mutex);
	}
	pthread_mutex_unlock(&client->mutex);
	return TRUE;
}

static void cb_call(rpc_conn *conn, rpc_code code, pointer output,
		size_t output_len, void* data) {
	rpc_response *rsp = data;
	rsp->code = code;
	rsp->output = output;
	rsp->output_len = output_len;

	uint64_t n = 1;
	write(POINTER_TO_INT(rsp->data), &n, sizeof(n));
}

rpc_code rpc_client_call(rpc_client* client, char* service_name,
		char* method_name, pointer input, size_t input_len, pointer* output,
		size_t* output_len) {
	int n_fd = eventfd(0, 0);
	rpc_response rsp;
	rsp.data = (INT_TO_POINTER(n_fd));
	rpc_client_call_async(client, service_name, method_name, input, input_len,
			cb_call, &rsp);

	//TODO
	//ev read timeout?
	fd_set r_set;
	FD_ZERO(&r_set);
	FD_SET(n_fd,&r_set);
	struct timeval tm;
	tm.tv_sec = TIMEOUT;
	tm.tv_usec = 0;
	int rt = select(n_fd + 1, &r_set, NULL, NULL, &tm);

	close(n_fd);
	if (rt == -1) {
		return RPC_Unkown;
	} else if (rt == 0) {
		return RPC_Timeout;
	} else {
		if (output != NULL)
			*output = rsp.output;
		if (output_len != NULL)
			*output_len = rsp.output_len;
		return rsp.code;
	}
}

void rpc_client_call_async(rpc_client* client, char* service_name,
		char* method_name, pointer input, size_t input_len,
		rpc_callback callback, pointer data) {
	rpc_request *req = rpc_request_new();
	req->callback = callback;
	req->method_name = method_name;
	req->service_name = service_name;
	req->input = input;
	req->input_len = input_len;
	req->data = data;

	int idx;
	idx = (client->last_thread + 1) % (client->thread_count);
	client->last_thread = idx;
	rpc_client_thread *th = client->threads + idx;
	rpc_async_queue_push(th->req_pending, req);
}
