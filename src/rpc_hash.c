/*
 * rpc_hash.c
 *
 *  Created on: 2011-3-26
 *      Author: yanghu
 */
#include "rpc_hash.h"

#include "hashtable/hashtable.h"

rpc_hash_table* rpc_hash_table_new_func(hash_func hash_func,
		equal_func key_equal_func) {
	struct hashtable *h;
	h = create_hashtable(16, hash_func, key_equal_func);
	return (rpc_hash_table*) h;
}

void rpc_hash_table_insert(rpc_hash_table *hash_table, pointer key,
		pointer value) {
	struct hashtable *h = (struct hashtable*) hash_table;
	hashtable_insert(h, key, value);
}

pointer rpc_hash_table_lookup(rpc_hash_table *hash_table, constpointer key) {
	struct hashtable *h = (struct hashtable*) hash_table;
	return hashtable_search(h, key);
}

boolean rpc_hash_table_remove(rpc_hash_table *hash_table, constpointer key) {
	struct hashtable *h = (struct hashtable*) hash_table;
	return hashtable_remove(h, key) != NULL;
}
