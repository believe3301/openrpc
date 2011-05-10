/*
 * rpc_sessionpool.h
 *
 *  Created on: 2011-3-23
 *      Author: yanghu
 */

#ifndef RPC_SESSIONPOOL_H_
#define RPC_SESSIONPOOL_H_

#include "rpc_types.h"

BEGIN_DECLS

typedef struct _rpc_sessionpool rpc_sessionpool;

rpc_sessionpool* rpc_sessionpool_new();

int rpc_sessionpool_insert(rpc_sessionpool *pool, pointer data);

pointer rpc_sessionpool_get(rpc_sessionpool *pool, int index);

boolean rpc_sessionpool_remove(rpc_sessionpool *pool, int index);

END_DECLS

#endif /* RPC_SESSIONPOOL_H_ */
