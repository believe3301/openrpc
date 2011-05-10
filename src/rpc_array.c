/*
 * rpc_array.c
 *
 *  Created on: 2011-3-24
 *      Author: yanghu
 */
#include "rpc_array.h"
#include <pthread.h>
#include <assert.h>

//dynamic pointer array

#define RPCARRAYSIZE 200

struct _rpc_array {
	pthread_mutex_t lock;
	pointer* data;
	int size;
	int cur;
};

rpc_array* rpc_array_new() {
	return rpc_array_new_size(RPCARRAYSIZE);
}

rpc_array* rpc_array_new_size(int size) {
	rpc_array *array = rpc_new(rpc_array,1);
	pthread_mutex_init(&array->lock, NULL);
	array->data = rpc_new0(pointer,size );
	array->size = size;
	array->cur = 0;
	return array;
}

pointer rpc_array_get(rpc_array* array) {
	assert(array);
	pointer data = NULL;
	pthread_mutex_lock(&array->lock);
	if (array->cur > 0) {
		data = array->data[--array->cur];
	}
	pthread_mutex_unlock(&array->lock);
	return data;
}

pointer rpc_array_index(rpc_array* array, int index) {
	assert(array);
	assert(index>=0);
	pointer data = NULL;
	pthread_mutex_lock(&array->lock);
	if (array->cur > index) {
		data = array->data[index];
	}
	pthread_mutex_unlock(&array->lock);
	return data;
}

boolean rpc_array_add(rpc_array* array, pointer data) {
	pthread_mutex_lock(&array->lock);
	if (array->cur < array->size) {
		array->data[array->cur++] = data;
	} else {
		size_t newsize = array->size * 2;
		pointer newdata = rpc_realloc(array->data, sizeof(pointer) * newsize);
		if (newdata) {
			array->data = newdata;
			array->size = newsize;
			array->data[array->cur++] = data;
		} else {
			pthread_mutex_unlock(&array->lock);
			return FALSE;
		}
	}
	pthread_mutex_unlock(&array->lock);
	return TRUE;
}
