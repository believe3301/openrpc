/*
 * rpc_util.h
 *
 *  Created on: 2011-3-26
 *      Author: yanghu
 */

#ifndef RPC_UTIL_H_
#define RPC_UTIL_H_

#include "rpc_types.h"
#include <stdarg.h>

char* rpc_vsprintf(char *format, ...);

int rpc_vslen(char const *format, va_list args);

int rpc_vasprintf(char **string, char const *format, va_list args);

void rpc_sleep(int ms);

void rpc_set_non_block(int fd);

#endif /* RPC_UTIL_H_ */
