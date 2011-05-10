/*
 * rpc_aync_queue.h
 *
 *  Created on: 2011-3-25
 *      Author: yanghu
 */

#ifndef RPC_ASYNC_QUEUE_H_
#define RPC_ASYNC_QUEUE_H_
#include "rpc_types.h"


BEGIN_DECLS

typedef struct _rpc_async_queue rpc_async_queue;

rpc_async_queue* rpc_async_queue_new();

void rpc_async_queue_free(rpc_async_queue *queue);

void rpc_async_queue_push(rpc_async_queue *queue, pointer data);

pointer rpc_async_queue_pop(rpc_async_queue *queue);

pointer rpc_async_queue_try_pop(rpc_async_queue *queue);

pointer rpc_async_queue_timed_pop(rpc_async_queue *queue, int ms);

END_DECLS

#endif /* RPC_ASYNC_QUEUE_H_ */
