#ifndef _OBJECT_POOL_H
#define _OBJECT_POOL_H

#if defined(OBJECT_POOL_MALLOC) ^ defined(OBJECT_POOL_FREE)
#error OBJECT_POOL_MALLOC and OBJECT_POOL_FREE both need to be defined in order to use custom allocators
#endif

#ifndef OBJECT_POOL_MALLOC
#include <stdlib.h>
#define OBJECT_POOL_MALLOC malloc
#define OBJECT_POOL_FREE free
#endif

#ifndef OBJECT_POOL_INDEX_TYPE
#define OBJECT_POOL_INDEX_TYPE size_t
#endif

typedef OBJECT_POOL_INDEX_TYPE object_pool_index_t;
typedef object_pool_index_t object_pool_count_t;

struct object_pool {
	char *block;
	object_pool_index_t head;
	object_pool_count_t count;
	size_t struct_size;
	size_t object_size;
};

typedef struct object_pool object_pool_t;

/**
 * object_pool_init(struct object_pool *pool, object_pool_count_t count, size_t object_size) - Initializes the object pool structure
 * @pool: Pointer of the pool structure. Must not be NULL.
 * @count: Object count to be allocated. Must be more than zero.
 * @object_size: Each object's size. Must be more than zero.
 *
 * Initializes the object pool structure, allocates the objects and sets the each object's headers.
 *
 * Return: Returns 0 on success. If arguments are invalid returns -EINVAL. If malloc fails, returns -ENOMEM.
 */
int object_pool_init(struct object_pool *pool, object_pool_count_t count, size_t object_size);

/**
 * object_pool_cleanup(struct object_pool *pool) - Cleans up the object pool structure
 * @pool: Pointer of the pool structure. Must not be NULL.
 *
 * Cleans up the object pool structure, resets it and frees the objects. Cleanup, does not require the objects to be released.
 * All acquired object pointers are invalid after cleanup.
 *
 * Return: Returns 0 on success. If arguments are invalid returns -EINVAL.
 */
int object_pool_cleanup(struct object_pool *pool);

/**
 * object_pool_acquire(struct object_pool *pool) - Acquires an object from the object pool
 * @pool: Pointer of the pool structure. Must not be NULL.
 *
 * Acquires an object from the object pool.
 *
 * Return: Returns the object acquired, on success. If the pool is exhausted, arguments are invalid or if an error occurs returns NULL.
 */
void *object_pool_acquire(struct object_pool *pool);

/**
 * object_pool_release(struct object_pool *pool, void *object) - Releases an acquired object from the object pool
 * @pool: Pointer of the pool structure. Must not be NULL.
 * @object: Pointer of the object acquired. Must not be NULL.
 * 
 * Releases an acquired object from the object pool. Double release is handled without error.
 * Object pointer is invalid after release.
 *
 * Return: Returns 0 on success. If arguments are invalid returns -EINVAL.
 */
int object_pool_release(struct object_pool *pool, void *object);

#endif /* _OBJECT_POOL_H */
