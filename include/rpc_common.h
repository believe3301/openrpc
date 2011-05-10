/*
 * rpc_common.h
 *
 *  Created on: 2011-3-20
 *      Author: yanghu
 */

#ifndef RPC_COMMON_H_
#define RPC_COMMON_H_

#include "rpc_types.h"

BEGIN_DECLS

#define WORKER_LEN 4
#define BACKLOG 5
#define BUFFER_SIZE 2048
#define RPC_VERSION "RPC/1.0"
#define TIMEOUT 120

typedef enum {
	RPC_Unkown = 0,
	RPC_OK = 200,
	RPC_BadRequest = 400,
	RPC_NotFound = 404,
	RPC_Timeout = 408,
	RPC_ServerError = 500
} rpc_code;

typedef enum {
	RPC_Parse_OK = 0, RPC_Parse_NeedData = 1, RPC_Parse_Error = 2
} rpc_parse_result;

typedef struct _rpc_server rpc_server;
typedef struct _rpc_client rpc_client;
typedef struct _rpc_conn rpc_conn;
typedef struct _rpc_request rpc_request;
typedef struct _rpc_response rpc_response;

typedef void (*rpc_callback)(rpc_conn *conn, rpc_code code, pointer output,
		size_t output_len, pointer data);

typedef void* (*rpc_thread_func)(void* args);

typedef void (*rpc_function)(rpc_conn *conn, pointer input, size_t input_len,
		pointer data);

typedef struct _rpc_dispatch_thread rpc_dispatch_thread;

typedef struct _rpc_client_thread rpc_client_thread;

typedef struct _rpc_worker_thread rpc_worker_thread;

struct _rpc_service {
	char *service_name;
	char *method_name;
	char *data;
	rpc_function cb;
};

typedef struct _rpc_service rpc_service;

const char* rpc_code_format(const rpc_code code);

rpc_code rpc_str_code(const char* str);

typedef struct token_s {
	char *value;
	size_t length;
} token_t;

int tokenize_command(char *command, token_t *tokens, const size_t max_token);

END_DECLS
#endif /* RPC_COMMON_H_ */
