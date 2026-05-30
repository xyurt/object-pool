/* object_pool by xyurt */

#ifndef _OBJECT_POOL_H
#define _OBJECT_POOL_H

/*  

	The object pool may work with threads but aren't fully thread-safe and not recommended

	To get the dynamic pool implementation, globally define _OBJECT_POOL_DYNAMIC_IMPL

*/

#include <stddef.h>

/* Do not modify the structure */
typedef struct object_pool_t {
	void *block;
	size_t head; /* Linked list's head's offset based on memory_block */
	size_t tail; /* Linked list's tail's offset based on memory_block */
	size_t last; /* Last object's offset based on memory_block */
	size_t count; /* Count of objects */
} object_pool;

/* Object pool has to be initialized before anything */
int object_pool_init(struct object_pool_t *pool);

/* Allocates the objects */
int object_pool_create(struct object_pool_t *pool, size_t object_count, size_t object_size);

/* Frees the objects | All handles and object pointers are invalid after freed | Objects needn't to be pushed back before freeing */
int object_pool_free(struct object_pool_t *pool);

/* Returns one if the pool is empty zero if not */
int object_pool_empty(struct object_pool_t *pool);

/* Returns the capacity of the pool */
size_t object_pool_capacity(struct object_pool_t *pool);

/* Returns the pool's first object's size, in bytes */
size_t object_pool_object_size(struct object_pool_t *pool);

/* Returns the pool's first object's size including the metadata, in bytes */
size_t object_pool_object_full_size(struct object_pool_t *pool);

/* Returns the pool's total memory usage, in bytes */
size_t object_pool_memory_footprint(struct object_pool_t *pool);

#ifndef _OBJECT_POOL_DYNAMIC_IMPL

/* Pop the next object's pointer | The pointer is owned by the pool and must not be changed | Reading and writing more than the object's size will corrupt the pool */
void *object_pool_pop(struct object_pool_t *pool);

#else

/* Get the object's pointer | The pointer is owned by the pool and must not be changed | Invalid handle will lead to corruption | Reading and writing more than the object's size will corrupt the pool */
void *object_pool_get(struct object_pool_t *pool, size_t object_handle);

/* Returns one if the handle is invalid zero if not */
int object_pool_handle_invalid(size_t object_handle);

/* Pop the next object's handle | Using an object handle from another pool is undefined behaviour */
size_t object_pool_pop(struct object_pool_t *pool);

/* Push the object back into the pool using its handle | Handle becomes invalid after push operation | Using invalid handles will lead to corruption */
int object_pool_push_handle(struct object_pool_t *pool, size_t object_handle);

/* Grow the pool dynamically | When grown, all pointers of the objects become invalid so handles must be used */
int object_pool_grow(struct object_pool_t *pool, size_t new_count);

#endif

/* Push the object back into the pool | Pushing the same object twice will lead to corruption | Object must be valid and allocated by a pool */
int object_pool_push(void *object);

#endif /* _OBJECT_POOL_H */