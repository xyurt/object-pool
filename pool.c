#include "pool.h"

#include <stdlib.h>
#include <string.h>

int object_pool_init(struct object_pool_t *pool) {
	if (pool == NULL) {
		return -1;
	}

	memset(pool, 0, sizeof(struct object_pool_t));

	pool->head = pool->tail = 1;

	return 0;
}

int object_pool_create(struct object_pool_t *pool, size_t object_count, size_t object_size) {
	size_t structure_size;
	size_t i;
	void *object_p;

	if (pool == NULL) {
		return -1;
	}

	/*

	struct:
		size_t length;
		void data[data_length];
		void *pool_ptr;
		size_t next_offset;

	*/

	structure_size = sizeof(size_t) + object_size + sizeof(void *) + sizeof(size_t);
	if (structure_size == 0) {
		return 0;
	}

	pool->block = malloc(object_count * structure_size);
	if (pool->block == NULL) {
		return -1;
	}

	pool->last = (object_count - 1) * structure_size;

	for (i = 0; i < object_count; i++) {
		object_p = ((char *)pool->block) + (i * structure_size);

		*((size_t *)object_p) = object_size;

		memcpy(((char *)object_p) + sizeof(size_t) + object_size, &pool, sizeof(void *));

		if (i != object_count - 1) {
			*(size_t *)(((char *)object_p) + sizeof(size_t) + object_size + sizeof(void *)) = (i + 1) * structure_size;
		}
		else {
			*(size_t *)(((char *)object_p) + sizeof(size_t) + object_size + sizeof(void *)) = pool->last + 1;
		}
	}

	pool->head = 0;
	pool->tail = pool->last;
	pool->count = object_count;

	return 0;
}
int object_pool_free(struct object_pool_t *pool) {
	if (pool == NULL) {
		return -1;
	}

	free(pool->block);
	pool->block = NULL;

	memset(pool, 0, sizeof(struct object_pool_t));
	pool->head = pool->tail = 1;

	return 0;
}

int object_pool_empty(struct object_pool_t *pool) {
	return pool == NULL || pool->block == NULL || pool->head > pool->last;
}

size_t object_pool_capacity(struct object_pool_t *pool) {
	return pool == NULL ? 0 : pool->count;
}

size_t object_pool_object_size(struct object_pool_t *pool) {
	return pool == NULL || pool->block == NULL ? 0 : *(size_t *)(pool->block);
}

size_t object_pool_object_full_size(struct object_pool_t *pool) {
	return pool == NULL || pool->block == NULL ? 0 : sizeof(size_t) + *(size_t *)(pool->block) + sizeof(void *) + sizeof(size_t);
}

size_t object_pool_memory_footprint(struct object_pool_t *pool) {
	if (pool == NULL) {
		return 0;
	}

	if (pool->block == NULL) {
		return sizeof(struct object_pool_t);
	}

	return pool->count * (sizeof(size_t) + *(size_t *)(pool->block) + sizeof(void *) + sizeof(size_t));
}

#ifndef _OBJECT_POOL_DYNAMIC_IMPL

void *object_pool_pop(struct object_pool_t *pool) {
	void *result;

	if (pool == NULL || pool->block == NULL) {
		return NULL;
	}

	if (pool->head > pool->last) {
		/* pool empty */
		return NULL;
	}

	result = ((char *)pool->block) + pool->head;

	pool->head = *(size_t *)(((char *)result) + sizeof(size_t) + (*(size_t *)(result)) + sizeof(void *));

	if (pool->head > pool->last) {
		pool->tail = pool->head;
	}

	return (void *)(((char *)result) + sizeof(size_t));
}

#else

void *object_pool_get(struct object_pool_t *pool, size_t object_handle) {
	if (pool == NULL || pool->block == NULL) {
		return NULL;
	}

	return ((char *)pool->block) + (object_handle - 1) + sizeof(size_t);
}

int object_pool_handle_invalid(size_t object_handle) {
	return object_handle == 0;
}

size_t object_pool_pop(struct object_pool_t *pool) {
	void *object_p;
	size_t result;

	if (pool == NULL || pool->block == NULL) {
		return 0;
	}

	if (pool->head > pool->last) {
		/* pool empty */
		return 0;
	}

	object_p = ((char *)pool->block) + pool->head;
	result = pool->head + 1;

	pool->head = *(size_t *)(((char *)object_p) + sizeof(size_t) + (*(size_t *)(object_p)) + sizeof(void *));

	if (pool->head > pool->last) {
		pool->tail = pool->head;
	}
	
	return result;
}

int object_pool_push_handle(struct object_pool_t *pool, size_t object_handle) {
	size_t object_size;
	size_t object;

	if (pool == NULL || pool->block == NULL || object_handle == 0) {
		return -1;
	}

	object_size = *(size_t *)(pool->block);

	object = ((char *)pool->block) + (object_handle - 1) + sizeof(size_t);

	memcpy((void *)&pool, ((char *)object) + object_size, sizeof(void *));

	*(size_t *)(((char *)object) + object_size + sizeof(void *)) = pool->head;
	pool->head = (size_t)((((char *)object) - sizeof(size_t)) - (char *)pool->block);

	return 0;
}

#endif

int object_pool_push(void *object) {
	size_t object_size;
	struct object_pool_t *pool;

	if (object == NULL) {
		return -1;
	}

	object_size = *(size_t *)(((char *)object) - sizeof(size_t));

	memcpy((void *)&pool, ((char *)object) + object_size, sizeof(void *));

	if (pool == NULL) {
		return -1;
	}

	*(size_t *)(((char *)object) + object_size + sizeof(void *)) = pool->head;
	pool->head = (size_t)((((char *)object) - sizeof(size_t)) - (char *)pool->block);

	return 0;
}

int object_pool_grow(struct object_pool_t *pool, size_t new_count) {
	size_t object_size;
	size_t structure_size;
	void *temp_block;
	size_t i;
	void *object_p;

	if (pool == NULL || pool->block == NULL || pool->count > new_count) {
		return -1;
	}

	if (pool->count == new_count) {
		return 0;
	}

	object_size = (*(size_t *)pool->block);
	structure_size = sizeof(size_t) + object_size + sizeof(void *) + sizeof(size_t);

	temp_block = realloc(pool->block, new_count * structure_size);
	if (temp_block == NULL) {
		return -1;
	}
	pool->block = temp_block;

	if (pool->head > pool->last) {
		pool->head = pool->count * structure_size;
	}

	if (pool->tail <= pool->last) {
		*(size_t *)(((char *)pool->block) + pool->tail + sizeof(size_t) + object_size + sizeof(void *)) = pool->count * structure_size;
	}

	pool->last = (new_count - 1) * structure_size;

	for (i = pool->count; i < new_count; i++) {
		object_p = ((char *)pool->block) + (i * structure_size);

		*((size_t *)object_p) = object_size;

		memcpy(((char *)object_p) + sizeof(size_t) + object_size, &pool, sizeof(void *));

		if (i != new_count - 1) {
			*(size_t *)(((char *)object_p) + sizeof(size_t) + object_size + sizeof(void *)) = (i + 1) * structure_size;
		}
		else {
			*(size_t *)(((char *)object_p) + sizeof(size_t) + object_size + sizeof(void *)) = pool->last + 1;
		}
	}

	pool->tail = pool->last;
	pool->count = new_count;

	return 0;
}