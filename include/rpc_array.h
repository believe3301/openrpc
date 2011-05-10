/*
 * rpc_array.h
 *
 *  Created on: 2011-3-24
 *      Author: yanghu
 */

#ifndef RPC_ARRAY_H_
#define RPC_ARRAY_H_

#include "rpc_types.h"

typedef struct _rpc_array rpc_array;

rpc_array* rpc_array_new();

rpc_array* rpc_array_new_size(int size);

pointer rpc_array_get(rpc_array* array);

pointer rpc_array_index(rpc_array* array, int index);

boolean rpc_array_add(rpc_array* array, pointer data);

#endif /* RPC_ARRAY_H_ */
