/* object_pool by xyurt */

#ifndef _OBJECT_POOL_H
#define _OBJECT_POOL_H

#include <stddef.h>

typedef size_t object_pool_index_t;
/*
- Internal pool index storage size
- this can be changed and changing this will change the overhead size and the maximum amount of objects in a pool
*/

typedef object_pool_index_t object_pool_count_t;

typedef size_t object_pool_size_t; /* Size_t alternative */

typedef unsigned char object_pool_handle_t;
/*
- unsigned char allows 256 different handle IDs (0–255), and they can be reused.
- this can be changed and changing this will change the overhead size
*/

typedef object_pool_handle_t object_pool;

/*
Creates an object pool
- Returns an object pool handle on success, zero on failure
- Arguments must be more than zero
*/
object_pool_handle_t object_pool_create(object_pool_count_t object_count, object_pool_size_t object_size);

/*
Destroys an object pool and invalidates the handle
- Returns 1 if success else 0
- Passing an invalid handle is undefined behaviour
*/
int object_pool_destroy(object_pool_handle_t handle);

/*
Pops an object from the pool
- Returns NULL if the pool is exhausted
- The object's ownership is strictly pool's
- Object must not be freed
- Read and write operations must be within the object bounds
- Passing an invalid handle is undefined behaviour
*/
void *object_pool_pop(object_pool_handle_t handle);

/*
Push an object back to the pool
- Invalidates the object pointer
*/
int object_pool_push(void *object);

/*
Returns the amount of objects that are waiting to be popped and free
*/
object_pool_count_t object_pool_free_count(object_pool_handle_t handle);

/*
Returns the amount of objects that are in use
*/
object_pool_count_t object_pool_active_count(object_pool_handle_t handle);

/*
Returns the total amount of objects
*/
object_pool_count_t object_pool_total_count(object_pool_handle_t handle);

/*
Returns the object size
*/
object_pool_size_t object_pool_object_size(object_pool_handle_t handle);

#endif /* _OBJECT_POOL_H */
