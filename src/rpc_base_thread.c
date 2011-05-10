/*
 * rpc_base_thread.c
 *
 *  Created on: 2011-3-25
 *      Author: yanghu
 */

//TODO
#include "rpc_base_thread.h"
#include "rpc_common.h"
#include <pthread.h>

struct _rpc_thread {
	pthread_t pid;
	rpc_thread_func func;
	void *data;
	boolean finished;
};

static void* rpc_thread_inner(void* args) {
	rpc_thread *th = args;
	//wait for start
	return th->func(th->data);
}

rpc_thread* rpc_thread_new(rpc_thread_func func) {
	rpc_thread *th = rpc_new(rpc_thread,1);
	th->func = func;
	pthread_t pid;
	if (pthread_create(&pid, NULL, rpc_thread_inner, th) == -1) {
		//go to
		rpc_free(th);
		return NULL;
	}
	th->pid = pid;
	th->finished = TRUE;
	return th;
}

void rpc_thread_start(rpc_thread* th, void* data) {
	//notify start
}

void rpc_thread_stop(rpc_thread* th) {
	//kill thread
}

void rpc_thread_free(rpc_thread* th) {
	//
}
