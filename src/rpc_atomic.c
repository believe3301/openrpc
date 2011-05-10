/*
 * rpc_atomic.c
 *
 *  Created on: 2011-4-11
 *      Author: yanghu
 */


#include "rpc_atomic.h"

int rpc_atomic_int_get(volatile int *atomic){
  __sync_synchronize();
  return *atomic;
}

void rpc_atomic_int_set(volatile int *atomic,int newval){
  *atomic = newval;
  __sync_synchronize();
}

void rpc_atomic_int_add(volatile int *atomic,int val){
  __sync_fetch_and_add(atomic,val);  
}

int rpc_atomic_int_exchange_and_add(volatile int *atomic,int val){
  return __sync_fetch_and_add(atomic,val);  
}

boolean rpc_atomic_int_compare_and_exchange(volatile int *atomic,int oldval,int newval){
  return __sync_bool_compare_and_swap(atomic,oldval,newval);
}

pointer rpc_atomic_pointer_get(volatile pointer *atomic){
  __sync_synchronize();
  return *atomic;
}

void rpc_atomic_pointer_set(volatile pointer *atomic,pointer newval){
  *atomic = newval;
  __sync_synchronize();
}

boolean rpc_atomic_pointer_compare_and_exchange(volatile pointer *atomic,pointer oldval,pointer newval){
  return __sync_bool_compare_and_swap(atomic,oldval,newval);
}
