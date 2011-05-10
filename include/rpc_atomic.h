/*
 * rpc_atomic.h
 *
 *  Created on: 2011-3-25
 *      Author: yanghu
 */

#ifndef RPC_ATOMIC_H_
#define RPC_ATOMIC_H_
#include "rpc_types.h"

int rpc_atomic_int_get(volatile int *atomic);

void rpc_atomic_int_set(volatile int *atomic,int newval);

void rpc_atomic_int_add(volatile int *atomic,int val);

#define rpc_atomic_int_inc(atomic) rpc_atomic_int_add(atomic,1)

#define rpc_atomic_int_dec_and_test(atomic) (rpc_atomic_int_exchange_and_add(atomic,-1)==-1)

int rpc_atomic_int_exchange_and_add(volatile int *atomic,int val);

boolean rpc_atomic_int_compare_and_exchange(volatile int *atomic,int oldval,int newval);

pointer rpc_atomic_pointer_get(volatile pointer *atomic);

void rpc_atomic_pointer_set(volatile pointer *atomic,pointer newval);

boolean rpc_atomic_pointer_compare_and_exchange(volatile pointer *atomic,pointer oldval,pointer newval);

#endif
