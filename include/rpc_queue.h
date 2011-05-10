/*
 * rpc_queue.h
 *
 *  Created on: 2011-3-22
 *      Author: yanghu
 */

#ifndef RPC_QUEUE_H_
#define RPC_QUEUE_H_

#include "rpc_types.h"

BEGIN_DECLS

typedef struct _rpc_queue rpc_queue;

typedef struct _rpc_queue_item rpc_queue_item;

struct _rpc_queue_item {
	void *data;
	struct _rpc_queue_item *next;
};

rpc_queue_item *rpc_queue_item_new();

void rpc_queue_item_free(rpc_queue_item *item);

rpc_queue* rpc_queue_new();

void rpc_queue_free(rpc_queue *q);

void rpc_queue_push(rpc_queue *q, void *data);

void* rpc_queue_pop(rpc_queue *q);

END_DECLS
#endif /* RPC_QUEUE_H_ */
