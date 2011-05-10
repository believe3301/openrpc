/*
 * rpc_request.c
 *
 *  Created on: 2011-3-21
 *      Author: yanghu
 */

#include "rpc_request.h"
#include "rpc_conn.h"
#include "rpc_array.h"
#include "rpc_util.h"
#include <assert.h>

static pthread_mutex_t freelock = PTHREAD_MUTEX_INITIALIZER;
static rpc_array *rpc_request_freelist = NULL;

rpc_request* rpc_request_new() {
	if (rpc_request_freelist == NULL) {
		pthread_mutex_lock(&freelock);
		if (rpc_request_freelist == NULL) {
			rpc_request_freelist = rpc_array_new();
		}
		pthread_mutex_unlock(&freelock);
	}
	//rpc_request *req = g_new(rpc_request,1);
	rpc_request *req = (rpc_request*) rpc_array_get(rpc_request_freelist);
	if (req == NULL) {
		req = rpc_new(rpc_request,1);
	}
	return req;
}

void rpc_request_free(rpc_request *req) {
	memset(req, 0, sizeof(rpc_request));
	if (!rpc_array_add(rpc_request_freelist, req)) {
		rpc_free(req);
	}
}

rpc_request* rpc_request_copy_head(rpc_request *req) {
	rpc_request *req_new = rpc_request_new();
	req_new->service_name = strdup(req->service_name);
	req_new->method_name = strdup(req->method_name);
	req_new->rpc_version = strdup(req->rpc_version);
	req_new->input_len = req->input_len;
	req_new->seq = req->seq;
	return req_new;
}

rpc_parse_result rpc_request_parse(rpc_conn *c, rpc_request **req) {
	//request line
	char *el, *cont, *ch;
	int avail, num;
	rpc_request *request = NULL;
	el = memchr(c->rcurr, '\n', c->rbytes);
	avail = c->rbytes;
	if (el) {
		cont = el + 1;
		avail -= (cont - c->rcurr);
		//windows newline separate is \r\n
		if ((el - c->rcurr) > 1 && *(el - 1) == '\r') {
			el--;
		}
		*el = '\0';
		token_t tokens[3];
		int n = tokenize_command(c->rcurr, tokens, 3);
		if (n != 3 && strcmp(tokens[0].value, "CALL") != 0) {
			fprintf(stderr, "request line begin error!\n");
			return RPC_Parse_Error;
		}
		ch = strchr(tokens[1].value, '@');
		if (!ch) {
			fprintf(stderr, "request line middler error!\n");
			return RPC_Parse_Error;
		}
		request = rpc_request_new();
		*ch = '\0';
		request->method_name = tokens[1].value;
		request->service_name = ch + 1;
		request->rpc_version = tokens[2].value;
		c->rcurr = cont;
		c->rbytes = avail;
	} else {
		fprintf(stderr, "not request line error!\n");
		return RPC_Parse_Error;
	}
	//seq
	el = memchr(c->rcurr, '\n', avail);
	if (el) {
		cont = el + 1;
		avail -= (cont - c->rcurr);
		if ((el - c->rcurr) > 1 && *(el - 1) == '\r') {
			el--;
		}
		*el = '\0';
		num = sscanf(c->rcurr, "seq:%d", &request->seq);
		c->rcurr = cont;
		c->rbytes = avail;
		if (num != 1) {
			rpc_request_free(request);
			fprintf(stderr, "seq parse error!\n");
			return RPC_Parse_Error;
		}
	} else {
		rpc_request_free(request);
		fprintf(stderr, "not seq line error!\n");
		return RPC_Parse_Error;
	}
	//body len
	el = memchr(c->rcurr, '\n', avail);
	if (el) {
		cont = el + 1;
		avail -= (cont - c->rcurr);
		if ((el - c->rcurr) > 1 && *(el - 1) == '\r') {
			el--;
		}
		*el = '\0';
		num = sscanf(c->rcurr, "body-len:%d", &request->input_len);
		c->rcurr = cont;
		c->rbytes = avail;
		if (num != 1) {
			rpc_request_free(request);
			fprintf(stderr, "body-len line parse error!\n");
			return RPC_Parse_Error;
		}
	} else {
		rpc_request_free(request);
		fprintf(stderr, "not body-len line error!\n");
		return RPC_Parse_Error;
	}
	//new line
	el = memchr(c->rcurr, '\n', avail);
	if (el) {
		cont = el + 1;
		avail -= (cont - c->rcurr);
		if ((el - c->rcurr == 0) || ((el - c->rcurr) == 1 && *(el - 1) == '\r')) {
			if (avail >= request->input_len) {
				request->input = cont;
				c->rcurr = cont + request->input_len;
				c->rbytes = avail - request->input_len;
				*req = request;
				return RPC_Parse_OK;
			} else {
				c->rcurr = cont;
				c->rbytes = avail;
				*req = request;
				return RPC_Parse_NeedData;
			}
		}
	}
	fprintf(stderr, "body error!\n");
	rpc_request_free(request);
	return RPC_Parse_Error;
}

void rpc_request_format(const rpc_request *request, rpc_conn *c) {
	assert(request !=NULL);
	assert(c != NULL);
	char *data;
	//request line
	data = rpc_vsprintf("CALL %s@%s %s\n", request->method_name,
			request->service_name, RPC_VERSION);
	rpc_conn_addiov(c, data, strlen(data));
	//seq
	data = rpc_vsprintf("seq:%d\n", request->seq);
	rpc_conn_addiov(c, data, strlen(data));
	//body len
	data = rpc_vsprintf("body-len:%d\n\n", request->input_len);
	rpc_conn_addiov(c, data, strlen(data));
	//body
	if (request->input)
		rpc_conn_addiov(c, request->input, request->input_len);
}
