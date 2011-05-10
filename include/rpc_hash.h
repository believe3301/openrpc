/*
 * rpc_hash.h
 *
 *  Created on: 2011-3-26
 *      Author: yanghu
 */

#ifndef RPC_HASH_H_
#define RPC_HASH_H_

#include "rpc_types.h"

BEGIN_DECLS

typedef struct _rpc_hash_table rpc_hash_table;

#define rpc_hash_table_new() rpc_hash_table_new_func(rpc_str_hash, rpc_str_equal)

rpc_hash_table* rpc_hash_table_new_func(hash_func hash_func,
		equal_func key_equal_func);

void rpc_hash_table_insert(rpc_hash_table *hash_table, pointer key,
		pointer value);

pointer rpc_hash_table_lookup(rpc_hash_table *hash_table, constpointer key);

boolean rpc_hash_table_remove(rpc_hash_table *hash_table, constpointer key);

boolean rpc_str_equal(constpointer v1, constpointer v2);

uint rpc_str_hash(constpointer v);

END_DECLS

#endif /* RPC_HASH_H_ */
