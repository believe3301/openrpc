/*
 * rpc_util.c
 *
 *  Created on: 2011-3-26
 *      Author: yanghu
 */
#define _GNU_SOURCE //for vasprint
#include "rpc_util.h"
#include <fcntl.h>
#include <sys/time.h>

char* rpc_vsprintf(char *format, ...) {
	va_list args;
	va_start(args, format);
	char *value;
	rpc_vasprintf(&value, format, args);
	va_end(args);
	return value;
}

int rpc_vslen(char const *format, va_list args) {
	char ch;
	size_t size = vsnprintf(&ch, 1, format, args) + 1;
	return size;
}

int rpc_vasprintf(char **string, char const *format, va_list args) {
	int len;
	len = vasprintf(string, format, args);
	if (len < 0) {
		*string = NULL;
	}
	return len;
}

void rpc_sleep(int ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000L * 1000L;
	nanosleep(&ts, NULL);
}

void rpc_set_non_block(int fd) {
	int flags;
	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
}

boolean rpc_str_equal(constpointer v1, constpointer v2) {
	const char *string1 = v1;
	const char *string2 = v2;

	return strcmp(string1, string2) == 0;
}

uint rpc_str_hash(constpointer v) {
	const signed char *p;
	uint h = 5381;
	for (p = v; *p != '\0'; p++)
		h = (h << 5) + h + *p;
	return h;
}
