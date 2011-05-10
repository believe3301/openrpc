/*
 * rpc_async_queue.c
 *
 *  Created on: 2011-3-25
 *      Author: yanghu
 */

#include "rpc_async_queue.h"
#include "rpc_queue.h"
#include <sys/time.h>

struct _rpc_async_queue {
	rpc_queue_item *head;
	rpc_queue_item *tail;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
//TODO atomic operation
//thread_wait
};

rpc_async_queue* rpc_async_queue_new() {
	rpc_async_queue *queue = rpc_new(rpc_async_queue,1);
	pthread_mutex_init(&queue->mutex, NULL);
	pthread_cond_init(&queue->cond, NULL);
	queue->head = NULL;
	queue->tail = NULL;
	return queue;
}

void rpc_async_queue_free(rpc_async_queue *queue) {
	pthread_mutex_lock(&queue->mutex);
	pointer data;
	while ((data = rpc_async_queue_try_pop(queue)) != NULL) {
		rpc_free(data);
	}
	pthread_mutex_unlock(&queue->mutex);
	rpc_free(queue);
}

void rpc_async_queue_push(rpc_async_queue *queue, pointer data) {
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
	pthread_cond_signal(&queue->cond);
	pthread_mutex_unlock(&queue->mutex);
}

static pointer rpc_asyn_queue_pop_inner(rpc_async_queue *queue, boolean try,
		struct timespec *tv) {
	rpc_queue_item *item = NULL;
	pthread_mutex_lock(&queue->mutex);
	item = queue->head;
	if (!try) {
		if (tv == NULL) {
			while ((item = queue->head) == NULL) {
				pthread_cond_wait(&queue->cond, &queue->mutex);
			}
		} else {
			while ((item = queue->head) == NULL) {
				if (!pthread_cond_timedwait(&queue->cond, &queue->mutex, tv)) {
					break;
				}
			}
			item = queue->head;
		}
	}
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

pointer rpc_async_queue_pop(rpc_async_queue *queue) {
	return rpc_asyn_queue_pop_inner(queue, FALSE, NULL);
}

pointer rpc_async_queue_try_pop(rpc_async_queue *queue) {
	return rpc_asyn_queue_pop_inner(queue, TRUE, NULL);
}

pointer rpc_async_queue_timed_pop(rpc_async_queue *queue, int ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000L * 1000L;
	return rpc_asyn_queue_pop_inner(queue, FALSE, &ts);
}
