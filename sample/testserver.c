/*
 * testserver.c
 *
 *  Created on: 2011-3-27
 *      Author: yanghu
 */
#include <stdlib.h>
#include <stdio.h>
#include "rpc.h"

void add(rpc_conn *conn, pointer input, size_t input_len, void* data) {
	int x, y;
	x = 0;
	y = 0;
	sscanf((char*) input, "%d %d", &x, &y);
	int z = x + y;
	printf("input is %s,len is %d,x is %d,y is %d,z is %d\n", (char*) input,
			input_len, x, y, z);
	//rpc_sleep(50);//simulate delay
	rpc_return(conn, &z, sizeof(z));
}

int main(int argc, char **argv) {
	rpc_server *server = rpc_server_create(8778);
	if (rpc_server_start(server)) {
		printf("start server\n");
	} else {
		printf("start server error\n");
	}

	rpc_server_regservice(server, "TestService", "add", add);
	for(;;){
		rpc_sleep(1000);
	}
	return EXIT_SUCCESS;
}
