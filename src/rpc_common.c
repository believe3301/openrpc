/*
 * rpc_common.c
 *
 *  Created on: 2011-3-21
 *      Author: yanghu
 */

#include "rpc_common.h"
#include <stdlib.h>
#include <string.h>

struct ec {
	int code;
	const char *name;
} err_codes[] = { { 0, "Unkown" }, { 200, "OK" }, { 400, "BadRequest" }, { 404,
		"NotFound" }, { 408, "Timeout" }, { 500, "ServerError" } };

#define nr_of_err (sizeof(err_codes)/sizeof(struct ec))

static int comp_code(const void *lhr, const void *rhr) {
	struct ec *mi1 = (struct ec*) lhr;
	struct ec *mi2 = (struct ec*) rhr;
	return strcmp(mi1->name, mi2->name);
}

static int comp_num(const void *lhr, const void *rhr) {
	struct ec *mi1 = (struct ec*) lhr;
	struct ec *mi2 = (struct ec*) rhr;
	return mi1->code - mi2->code;
}

const char* rpc_code_format(const rpc_code code) {
	struct ec key, *res;
	key.code = code;
	res = bsearch(&key, err_codes, nr_of_err, sizeof(struct ec), comp_num);
	if (res) {
		return res->name;
	}
	return NULL;
}

rpc_code rpc_str_code(const char* str) {
	qsort(err_codes, nr_of_err, sizeof(struct ec), comp_code);
	struct ec key, *res;
	key.name = str;
	res = bsearch(&key, err_codes, nr_of_err, sizeof(struct ec), comp_code);
	if (res) {
		return res->code;
	} else {
		return RPC_Unkown;
	}
}

int tokenize_command(char *command, token_t *tokens, const size_t max_token) {
	char *b, *e;
	size_t ntokens = 0;
	for (b = e = command; ntokens < max_token; ++e) {
		if (*e == ' ') {
			if (b != e) {
				tokens[ntokens].value = b;
				tokens[ntokens].length = e - b;
				ntokens++;
				*e = '\0';
			}
			b = e + 1;
		} else if (*e == '\0') {
			if (b != e) {
				tokens[ntokens].value = b;
				tokens[ntokens].length = e - b;
				ntokens++;
			}
			break;
		}
	}
	return ntokens;
}
