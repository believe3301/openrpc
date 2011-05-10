/*
 * rpc_base_thread.h
 *
 *  Created on: 2011-3-25
 *      Author: yanghu
 */

#ifndef RPC_BASE_THREAD_H_
#define RPC_BASE_THREAD_H_

//one rpc_thread corresponding to underlying op thread

typedef struct _rpc_thread rpc_thread;

rpc_thread* rpc_thread_new();

void rpc_thread_start(rpc_thread* th, void* data);

void rpc_thread_stop(rpc_thread* th);

void rpc_thread_free(rpc_thread* th);

#endif /* RPC_BASE_THREAD_H_ */
