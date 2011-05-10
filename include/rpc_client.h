/*
 * rpc_client.h
 *
 *  Created on: 2011-3-21
 *      Author: yanghu
 */

#ifndef RPC_CLIENT_H_
#define RPC_CLIENT_H_

#include "rpc_common.h"

#define CLIENT_THREAD_NUM 1
struct _rpc_client {
	char *host;
	int port;
	rpc_client_thread *threads;
	int thread_count;
	int last_thread;

	int init_count;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
};

rpc_client* rpc_client_new();

boolean rpc_client_connect(rpc_client* client, char* host, int port);

rpc_code rpc_client_call(rpc_client* client, char* service_name,
		char* method_name, pointer input, size_t input_len, pointer* output,
		size_t* output_len);

void rpc_client_call_async(rpc_client* client, char* service_name,
		char* method_name, pointer input, size_t input_len,
		rpc_callback callback, pointer data);
#endif /* RPC_CLIENT_H_ */
