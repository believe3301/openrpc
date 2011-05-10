#include <stdlib.h>
#include <stdio.h>
#include "rpc.h"
void cb_add(rpc_conn *conn, rpc_code code, pointer output, size_t output_len,
		void* data) {
	if (code != RPC_OK) {
		fprintf(stderr, "call error %s\n", rpc_code_format(code));
		exit(1);
	}
	int result = *(int*) output;
	int i = POINTER_TO_INT(data);
	if (result != (2 * i + 1)) {
		fprintf(stderr, "i is %d,result:%d should %d\n", i, result, 2 * i + 1);
		exit(1);
	}
}

int main(int argc , char ** argv){
	printf("test client begin!\n");	
	rpc_client *client = rpc_client_new();
	rpc_client_connect(client,"127.0.0.1",8778);
	printf("test client started success!");
	
	pointer output;
	size_t len;
	char input[256];
	int i, result;

	rpc_code code;

	for (i = 0; i < 10000; i++) {
		len = sprintf(input, "%d %d", i, i + 1);
		printf("call!\n");
		code = rpc_client_call(client, "TestService", "add", input, strlen(
				input) + 1, &output, &len);
		if (code != RPC_OK) {
			printf("call error %s\n", rpc_code_format(code));
			exit(1);
		}
		result = *(int*) output;
		if (result != (2 * i + 1)) {
			printf("i is %d,result:%d should %d\n", i, result, 2 * i + 1);
			exit(1);
		}
		printf("result:%d\n", result);
	}
	int j;
	for (j = 0; j < 10; j++) {
		for (i = 0; i < 10000; i++) {
			len = sprintf(input, "%d %d", i, i + 1);
			rpc_client_call_async(client, "TestService", "add", strdup(input),
					strlen(input) + 1, cb_add, INT_TO_POINTER(i));
		}
    rpc_sleep(1000);
	}

	printf("test ok\n");
	for (;;) {
		rpc_sleep(1000);
	}
	return EXIT_SUCCESS;
}

