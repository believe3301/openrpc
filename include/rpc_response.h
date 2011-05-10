/*
 * rpc_response.h
 *
 *  Created on: 2011-3-24
 *      Author: yanghu
 */

#ifndef RPC_RESPONSE_H_
#define RPC_RESPONSE_H_

#include "rpc_common.h"

struct _rpc_response {
	rpc_code code;
	char *phrase;
	int seq;
	pointer output;
	size_t output_len;
	pointer data;
};

rpc_response* rpc_response_new();

void rpc_response_free(rpc_response *rsp);

rpc_response* rpc_response_copy_head(rpc_response *rsp);

rpc_parse_result rpc_response_parse(rpc_conn *c, rpc_response **rsp);

#endif /* RPC_RESPONSE_H_ */
