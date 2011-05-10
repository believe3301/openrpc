/*
 * rpc_sessionpool.c
 *
 *  Created on: 2011-3-23
 *      Author: yanghu
 */
#include "rpc_sessionpool.h"
#include "rpc_queue.h"
#include <assert.h>

#define POOLCAPACITY 1000

struct _rpc_sessionpool {
	pthread_mutex_t mutex;
	pointer *data;
	int size;
};

rpc_sessionpool* rpc_sessionpool_new() {
	rpc_sessionpool *pool = rpc_new(rpc_sessionpool,1);
	pthread_mutex_init(&pool->mutex, NULL);
	pool->data = rpc_new0(pointer,POOLCAPACITY);
	pool->size = POOLCAPACITY;
	return pool;
}

int rpc_sessionpool_insert(rpc_sessionpool *pool, pointer data) {
	pthread_mutex_lock(&pool->mutex);
	int i;
	for (i = 0; i < pool->size; ++i) {
		if (!pool->data[i]) {
			pool->data[i] = data;
			pthread_mutex_unlock(&pool->mutex);
			return i;
		}
	}
	int old_size = pool->size;
	int new_size = old_size * 2;
	pointer new_data = rpc_realloc(pool->data, sizeof(pointer) * new_size);
	if (!new_data) {
    fprintf(stderr,"insert session pool error!\n");
    return -1;
	}
	pool->data = new_data;
	pool->data[old_size] = data;
	pool->size = new_size;
	pthread_mutex_unlock(&pool->mutex);
	return old_size;
}

pointer rpc_sessionpool_get(rpc_sessionpool *pool, int index) {
	assert(index >= 0);
	assert(index<pool->size);
	pointer data;
	pthread_mutex_lock(&pool->mutex);
	data = pool->data[index];
	pthread_mutex_unlock(&pool->mutex);
	return data;
}

boolean rpc_sessionpool_remove(rpc_sessionpool *pool, int index) {
	pthread_mutex_lock(&pool->mutex);
	if (pool->data[index] != NULL) {
		pool->data[index] = NULL;
	}
	pthread_mutex_unlock(&pool->mutex);
	return TRUE;
}
