/* object_pool by xyurt */

#ifndef _OBJECT_POOL_H
#define _OBJECT_POOL_H

#include <stddef.h>

/*
 
Changing the types of the internal overhead variables (changes the overhead size)

Use this globally or in this header file:
#define OBJECT_POOL_TYPE_MODIFY
#define OBJECT_POOL_INDEX_TYPE size_t
#define OBJECT_POOL_HANDLE_TYPE unsigned char

The OBJECT_POOL_INDEX_TYPE definition value sets the max amount of objects a pool can theoretically have
Changing this will change the first argument type of object_pool_create() 
if you set it to size_t you can have up to (SIZE_MAX - 1) objects in a single pool
if you set it to unsigned long you can have up to (ULONG_MAX - 1) objects in a single pool

The OBJECT_POOL_HANDLE_TYPE definition value sets the max amount of pools that can be created, the pool handles are reused after object_pool_destroy() 
if you set it to size_t you can have up to (SIZE_MAX - 1) pools
if you set it to unsigned long you can have up to (ULONG_MAX - 1) pools

*/

#ifndef OBJECT_POOL_TYPE_MODIFY 
#define OBJECT_POOL_INDEX_TYPE size_t
#define OBJECT_POOL_HANDLE_TYPE unsigned char
#else
#ifndef OBJECT_POOL_INDEX_TYPE
#error OBJECT_POOL_INDEX_TYPE must be defined
#endif

#ifndef OBJECT_POOL_HANDLE_TYPE
#error OBJECT_POOL_HANDLE_TYPE must be defined
#endif
#endif

typedef OBJECT_POOL_INDEX_TYPE object_pool_index_t;
typedef OBJECT_POOL_HANDLE_TYPE object_pool_handle_t;

typedef object_pool_index_t object_pool_count_t;
typedef size_t object_pool_size_t;

typedef object_pool_handle_t object_pool;
typedef const void *object_pool_ptr;

#define OBJECT_POOL_INVALID (0)

object_pool_handle_t object_pool_create(object_pool_count_t object_count, object_pool_size_t object_size);

/*
Destroys an object pool and its object pointers and invalidates the handle
- Returns 1 if success else 0
- Passing an invalid handle is undefined behaviour
- Using an object pointer belonging to a destroyed pool after destroy is undefined behaviour
*/
int object_pool_destroy(object_pool_handle_t handle);

/*
Returns the raw pointer of the object pool structure
- Must only be used as the arguments of object_pool_ptr_pop and object_pool_ptr_push
- Using the functions that use the raw pointers are faster
*/
object_pool_ptr object_pool_from_handle(object_pool_handle_t handle);

/*
Pops an object from the pool
- Returns NULL if the pool is exhausted
- The object's ownership is strictly pool's
- Object must not be freed
- Read and write operations must be within the object bounds
- Passing an invalid pool pointer is undefined behaviour
*/
void *object_pool_ptr_pop(object_pool_ptr pool_ptr);

/*
Push an object back to the pool
- Invalidates the object pointer
- Passing an invalid pool pointer or an object pointer is undefined behaviour
*/
int object_pool_ptr_push(object_pool_ptr pool_ptr, void *object);

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
- Passing an invalid object pointer is undefined behaviour
*/
int object_pool_push(void *object);

/*
Returns the pool handle of the object
- Returns 0 on failure
*/
object_pool_handle_t object_pool_owns(const void *object);

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

/*
Returns the overhead size of one object in a pool
*/
object_pool_size_t object_pool_overhead_size();

#endif /* _OBJECT_POOL_H */
