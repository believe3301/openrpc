/*
 * rpc_request.h
 *
 *  Created on: 2011-3-21
 *      Author: yanghu
 */

#ifndef RPC_REQUEST_H_
#define RPC_REQUEST_H_

#include "rpc_common.h"

struct _rpc_request {
	char* service_name;
	char* method_name;
	char* rpc_version;
	int seq;
	pointer input;
	size_t input_len;
	pointer data;
	rpc_callback callback;
};

rpc_request* rpc_request_new();

void rpc_request_free(rpc_request *req);

rpc_request* rpc_request_copy_head(rpc_request *req);

rpc_parse_result rpc_request_parse(rpc_conn *c, rpc_request **req);

void rpc_request_format(const rpc_request *request, rpc_conn *c);

#endif /* RPC_REQUEST_H_ */
