/*
 * rpc_types.h
 *
 *  Created on: 2011-3-25
 *      Author: yanghu
 */

/*
 * same type reference glib
 * 3q for glib
 */

#ifndef RPC_TYPES_H_
#define RPC_TYPES_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#ifdef  __cplusplus
# define BEGIN_DECLS  extern "C" {
# define END_DECLS    }
#else
# define BEGIN_DECLS
# define END_DECLS
#endif

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#define POINTER_TO_INT(p) ((int)  (long) (p))
#define INT_TO_POINTER(i) ((pointer) (long) (i))

BEGIN_DECLS

typedef int boolean;
typedef unsigned int uint;
typedef void* pointer;
typedef const void *constpointer;

typedef boolean (*equal_func)(constpointer a, constpointer b);
typedef uint (*hash_func)(constpointer key);

#define rpc_free(mem) free(mem)

#define rpc_new(struct_type, n_structs)  ((struct_type *) malloc((n_structs)*sizeof (struct_type)))

#define rpc_new0(struct_type, n_structs)  ((struct_type *) calloc(1,(n_structs)*sizeof (struct_type)))

#define rpc_realloc(mem,n_bytes) realloc(mem, n_bytes)
END_DECLS
#endif /* RPC_TYPES_H_ */
