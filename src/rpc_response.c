/*
 * rpc_request.c
 *
 *  Created on: 2011-3-21
 *      Author: yanghu
 */

#include "rpc_response.h"
#include "rpc_conn.h"
#include "rpc_array.h"
#include "rpc_util.h"
#include <assert.h>

static pthread_mutex_t freelock = PTHREAD_MUTEX_INITIALIZER;

static rpc_array *rpc_response_freelist = NULL;

rpc_response* rpc_response_new() {
	if (rpc_response_freelist == NULL) {
		pthread_mutex_lock(&freelock);
		if (rpc_response_freelist == NULL) {
			rpc_response_freelist = rpc_array_new();
		}
		pthread_mutex_unlock(&freelock);
	}
	//rpc_response *rsp = g_new(rpc_response,1);
	rpc_response *rsp = (rpc_response*) rpc_array_get(rpc_response_freelist);
	if (rsp == NULL) {
		rsp = rpc_new(rpc_response,1);
	}
	return rsp;
}

void rpc_response_free(rpc_response *rsp) {
	memset(rsp, 0, sizeof(rpc_response));
	if (!rpc_array_add(rpc_response_freelist, rsp)) {
		rpc_free(rsp);
	}
}

rpc_response* rpc_response_copy_head(rpc_response *rsp) {
	rpc_response *rsp_new = rpc_response_new();
	rsp_new->code = rsp->code;
	rsp_new->seq = rsp->seq;
	rsp_new->data = rsp->data;
	rsp_new->output_len = rsp->output_len;
	rsp_new->phrase = strdup(rsp->phrase);
	return rsp_new;
}

rpc_parse_result rpc_response_parse(rpc_conn *c, rpc_response **rsp) {
	//request line
	char *el, *cont;
	int avail, num;
	rpc_response *response = NULL;
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
		if (n != 3 && strcmp(tokens[0].value, RPC_VERSION) != 0) {
			fprintf(stderr, "status line begin error!\n");
			return RPC_Parse_Error;
		}
		response = rpc_response_new();
		response->code = (int) strtol(tokens[1].value, NULL, 10);
		response->phrase = tokens[2].value;
		c->rcurr = cont;
		c->rbytes = avail;
	} else {
		fprintf(stderr, "not status line error!\n");
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
		num = sscanf(c->rcurr, "seq:%d", &response->seq);
		c->rcurr = cont;
		c->rbytes = avail;
		if (num != 1) {
			rpc_response_free(response);
			fprintf(stderr, "seq parse error!\n");
			return RPC_Parse_Error;
		}
	} else {
		rpc_response_free(response);
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
		num = sscanf(c->rcurr, "body-len:%d", &response->output_len);
		c->rcurr = cont;
		c->rbytes = avail;
		if (num != 1) {
			rpc_response_free(response);
			fprintf(stderr, "body-len line parse error!\n");
			return RPC_Parse_Error;
		}
	} else {
		rpc_response_free(response);
		fprintf(stderr, "not body-len line error!\n");
		return RPC_Parse_Error;
	}
	//new line
	el = memchr(c->rcurr, '\n', avail);
	if (el) {
		cont = el + 1;
		avail -= (cont - c->rcurr);
		if ((el - c->rcurr == 0) || ((el - c->rcurr) == 1 && *(el - 1) == '\r')) {
			if (avail >= response->output_len) {
				response->output = cont;
				c->rcurr = cont + response->output_len;
				c->rbytes = avail - response->output_len;
				*rsp = response;
				return RPC_Parse_OK;
			} else {
				c->rcurr = cont;
				c->rbytes = avail;
				*rsp = response;
				return RPC_Parse_NeedData;
			}
		}
	}
	fprintf(stderr, "body error!\n");
	rpc_response_free(response);
	return RPC_Parse_Error;
}
