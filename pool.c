#include "pool.h"

#include <string.h>
#include <stddef.h>
#include <errno.h>

#define ALIGNMENT (sizeof(void *))
#define ALIGN(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#ifdef OBJECT_POOL_NO_ALIGN
#define ALIGN_ENABLED 0
#else
#define ALIGN_ENABLED 1
#endif

static void object_pool_offsets_init(struct object_pool *pool);

int object_pool_init(struct object_pool *pool, size_t count, size_t object_size)
{
	size_t struct_size = sizeof(size_t) + object_size;

	if (ALIGN_ENABLED) {
		struct_size = ALIGN(struct_size);
	}

	if (!pool || count == 0 || object_size == 0)
		return -EINVAL;

	pool->block = OBJECT_POOL_MALLOC(count * struct_size);
	if (!pool->block)
		return -ENOMEM;

	pool->count = count;
	pool->struct_size = struct_size;

	object_pool_offsets_init(pool);

	pool->object_size = object_size;

	return 0;
}

int object_pool_cleanup(struct object_pool *pool) 
{
	if (!pool)
		return -EINVAL;

	OBJECT_POOL_FREE(pool->block);
	pool->block = NULL;
	pool->head = 0;
	
	pool->count = 0;
	pool->struct_size = 0;
	pool->object_size = 0;

	return 0;
}

void *object_pool_acquire(struct object_pool *pool) 
{
	size_t head;
	size_t struct_size;
	size_t idx;
	char *target;
	size_t tmp;

	if (!pool)
		return NULL;

	head = pool->head;
	if (head == 0)
		return NULL;

	idx = head - 1;

	target = pool->block + idx * pool->struct_size;

	pool->head = *(size_t *)target;
	*(size_t *)target = head;

	return target + sizeof(size_t); /* result is the object, excluding the header */
}

int object_pool_release(struct object_pool *pool, void *object) 
{
	size_t *offset;
	size_t tmp;

	if (!pool || !object) {
		return -EINVAL;
	}

	offset = (size_t *)((char *)object - sizeof(size_t));

	tmp = pool->head;
	pool->head = *offset;
	*offset = tmp;

	return 0;
}

static void object_pool_offsets_init(struct object_pool *pool)
{
	size_t i = 0;
	size_t count = pool->count;
	char *block = pool->block;
	size_t struct_size = pool->struct_size;
	size_t tmp;

	for (;;) {
		if (i == count)
			break;

		if (i + 1 == count) {
			tmp = 0;
		}
		else {
			tmp = i + 1 + 1; /* zero was reserved so next handle */
		}

		*(size_t *)(block + i * struct_size) = tmp;

		i++;
	}

	pool->head = 1;
}
