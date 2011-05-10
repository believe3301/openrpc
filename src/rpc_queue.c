/*
 * rpc_queue.c
 *
 *  Created on: 2011-3-22
 *      Author: yanghu
 */

#include "rpc_queue.h"
#include <pthread.h>

#define FREE_ITEM_LEN 32


struct _rpc_queue {
	rpc_queue_item *head;
	rpc_queue_item *tail;
	pthread_mutex_t mutex;
};

static pthread_mutex_t free_list_locker = PTHREAD_MUTEX_INITIALIZER;
static rpc_queue_item *free_item_list;

rpc_queue_item *rpc_queue_item_new() {
	rpc_queue_item *item = NULL;
	if (free_item_list != NULL) {
		pthread_mutex_lock(&free_list_locker);
		if (free_item_list != NULL) {
			item = free_item_list;
			free_item_list = free_item_list->next;
		}
		pthread_mutex_unlock(&free_list_locker);
	}
	if (item == NULL) {
		item = rpc_new(rpc_queue_item,FREE_ITEM_LEN);
		if (item == NULL)
			return NULL;
		int i;
		for (i = 2; i < FREE_ITEM_LEN; i++) {
			item[i - 1].next = item + i;
		}
		pthread_mutex_lock(&free_list_locker);
		item[FREE_ITEM_LEN - 1].next = free_item_list;
		free_item_list = item + 1;
		pthread_mutex_unlock(&free_list_locker);
	}
	return item;
}

void rpc_queue_item_free(rpc_queue_item *item) {
	pthread_mutex_lock(&free_list_locker);
	if (free_item_list != NULL) {
		item->next = free_item_list;
	} else {
		item ->next = NULL;
	}
	free_item_list = item;
	pthread_mutex_unlock(&free_list_locker);
}

rpc_queue *rpc_queue_new() {
	rpc_queue *q = rpc_new(rpc_queue,1);
	q->head = NULL;
	q->tail = NULL;
	pthread_mutex_init(&q->mutex, NULL);
	return q;
}

void *rpc_queue_pop(rpc_queue *queue) {
	rpc_queue_item *item = NULL;
	pthread_mutex_lock(&queue->mutex);
	item = queue->head;
	if (item != NULL) {
		queue->head = item->next;
		if (item->next == NULL) {
			queue->tail = NULL;
		}
	}
	pthread_mutex_unlock(&queue->mutex);
	if (item) {
		rpc_queue_item_free(item);
		return item->data;
	} else {
		return NULL;
	}
}

void rpc_queue_push(rpc_queue *queue, void *data) {
	rpc_queue_item *item = rpc_queue_item_new();
	item->data = data;
	item->next = NULL;
	pthread_mutex_lock(&queue->mutex);
	if (queue->tail != NULL) {
		queue->tail->next = item;
	} else {
		queue->head = item;
	}
	queue->tail = item;
	pthread_mutex_unlock(&queue->mutex);
}
