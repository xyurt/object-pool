#include "pool.h"

#include <stdlib.h>
#include <string.h>

#define MAX_TOTAL_MEMORY_FOR_STRUCTURES (SIZE_MAX - sizeof(struct object_pool_t))

#define MEMORY_FOR_STRUCTURE_HEADERS (sizeof(object_pool_handle_t) + sizeof(object_pool_index_t))
#define MAX_MEMORY_FOR_OBJECT_DATA (SIZE_MAX - MEMORY_FOR_STRUCTURE_HEADERS)

#define MEMORY_FOR_STRUCTURE(object_size) (MEMORY_FOR_STRUCTURE_HEADERS + (size_t)(object_size))

struct object_pool_t {
	struct object_pool_t *next;			  /* Object pool linked list next pointer */
	struct object_pool_t *prev;			  /* Object pool linked list previous pointer */
	object_pool_handle_t handle;		  /* Object pool handle */
	object_pool_index_t head; 	   		  /* Objects' linked list's head's index based on the memory_block */
	object_pool_index_t free_count;	   	  /* Amount of objects that can be popped */
	object_pool_index_t active_count;     /* Amount of objects that is in use */
	object_pool_index_t total_count;	  /* Total objects in the memory block */
	object_pool_size_t object_size;    	  /* One object's size */
	size_t structure_size;	  	   		  /* Whole structure size of one object takes in the memory block */
	/*void block[];					      /* Memory block where objects are stored in, not in the struct but available in memory */
};

struct object_pool_free_handle_t {
	struct object_pool_free_handle_t *next;
	object_pool_handle_t handle;
};

struct object_pool_vector_t {
	void *data;
	object_pool_handle_t capacity;
	object_pool_handle_t count;
	struct object_pool_free_handle_t *free_handle_list;
};

struct object_pool_vector_t _object_pool_vector;
struct object_pool_t *last_pool;

/*

Index zero is reserved for NULL check

struct:
	object_pool_handle_t pool_handle;
	object_pool_index_t next_idx; /* When popped next_idx will point to the struct's index instead of the next, this alone gives 2-3x performance than to do division on block
	void data[data_length];

*/

char object_pool_create_will_overflow(object_pool_count_t object_count, object_pool_size_t object_size, size_t *structure_size) {
	size_t max_object_count;
	size_t structure_size_temp;

	if (object_count <= 0 || object_size <= 0) {
		/* Object count or object size cannot be lower than or equal to zero */
		return 1;
	}

	if (object_size > MAX_MEMORY_FOR_OBJECT_DATA) {
		/* Object structure size overflows */
		return 1;
	}

	/* Memory needed for one object structure */
	structure_size_temp = MEMORY_FOR_STRUCTURE(object_size);

	max_object_count = MAX_TOTAL_MEMORY_FOR_STRUCTURES / structure_size_temp;
	if (max_object_count == 0) {
		/* Total structure size exceeds the limit */
		return 1;
	}
	max_object_count -= 1; /* Index zero for structures is reserved */

	if (object_count > max_object_count) {
		/* Object count overflows the size required to call malloc() */
		return 1;
	}

	*structure_size = structure_size_temp;

	return 0;
}

char object_pool_global_list_grow() {
	/* Could've only used realloc but maybe the system doesn't accept zero as an argument so splitted into two ifs */
	if (_object_pool_vector.data == NULL) {
		_object_pool_vector.data = malloc(sizeof(void *));
		if (_object_pool_vector.data == NULL) {
			return 0;
		}

		_object_pool_vector.capacity = 1;
	}
	else {
		if (_object_pool_vector.count >= _object_pool_vector.capacity) {
			void *new_block;
			size_t global_vector_new_capacity;
			if ((size_t)(_object_pool_vector.capacity) > (SIZE_MAX >> 1)) {
				if ((size_t)(_object_pool_vector.capacity) < SIZE_MAX) {
					global_vector_new_capacity = _object_pool_vector.capacity + 1;
				}
				else {
					return 0;
				}
			}
			else {
				global_vector_new_capacity = ((size_t)_object_pool_vector.capacity) << 1;
			}

			if (global_vector_new_capacity == SIZE_MAX) {
				global_vector_new_capacity = SIZE_MAX - 1;

				if (_object_pool_vector.capacity == global_vector_new_capacity) {
					return 0;
				}
			}

			new_block = realloc(_object_pool_vector.data, sizeof(void *) * global_vector_new_capacity);
			if (new_block == NULL) {
				return 0;
			}
			_object_pool_vector.data = new_block;

			_object_pool_vector.capacity = global_vector_new_capacity;
		}
	}

	return 1;
}

void object_pool_global_list_get_free_handle(object_pool_handle_t *out) {
	if (_object_pool_vector.free_handle_list == NULL) {
		*out = _object_pool_vector.count + 1;
		return;
	}

	struct object_pool_free_handle_t *next_handle;

	next_handle = _object_pool_vector.free_handle_list;
	*out = next_handle->handle;

	_object_pool_vector.free_handle_list = next_handle->next;

	free(next_handle);
}

char object_pool_global_list_add(struct object_pool_t *pool) {
	object_pool_handle_t handle;

	if (pool == NULL || !object_pool_global_list_grow()) {
		return 0;
	}

	object_pool_global_list_get_free_handle(&handle);

	*(void **)(((char *)_object_pool_vector.data) + ((handle - 1) * sizeof(void *))) = pool;
	pool->handle = handle;

	_object_pool_vector.count++;

	return 1;
}

char object_pool_create_initialize_objects(struct object_pool_t *pool) {
	char *object_p;
	object_pool_count_t i;

	if (pool == NULL) {
		return 0;
	}

	object_p = ((char *)pool) + sizeof(struct object_pool_t);
	for (i = 0; i < pool->total_count; i++) {

		*((object_pool_handle_t *)object_p) = pool->handle;
		*((object_pool_index_t *)(((char *)object_p) + sizeof(object_pool_handle_t))) = i == pool->total_count - 1 ? 2 : i + 2; /* Index zero is reserved for NULL check */

		object_p += pool->structure_size;
	}

	pool->head = 1;

	pool->free_count = pool->total_count;

	return 1;
}

void object_pool_global_list_add_free_handle(object_pool_handle_t handle) {
	struct object_pool_free_handle_t *handle_struct = (struct object_pool_free_handle_t *)malloc(sizeof(struct object_pool_free_handle_t));
	if (handle_struct == NULL) {
		return;
	}

	handle_struct->handle = handle;

	handle_struct->next = _object_pool_vector.free_handle_list;
	_object_pool_vector.free_handle_list = handle_struct;
}

void object_pool_global_list_remove(object_pool_handle_t handle) {
	struct object_pool_free_handle_t *next_handle;

	if (handle != _object_pool_vector.count) {
		object_pool_global_list_add_free_handle(handle);
	}

	_object_pool_vector.count--;

	if (_object_pool_vector.count == 0) {
		for (;;) {
			if (_object_pool_vector.free_handle_list == NULL) {
				break;
			}

			next_handle = _object_pool_vector.free_handle_list->next;
			free(_object_pool_vector.free_handle_list);
			_object_pool_vector.free_handle_list = next_handle;
		}
	}
}

void *object_pool_get_block(struct object_pool_t *pool) {
	if (pool == NULL) {
		return NULL;
	}

	return ((char *)pool) + sizeof(struct object_pool_t);
}

void *object_pool_get_from_index(struct object_pool_t *pool, object_pool_index_t object_idx) {
	if (pool == NULL || object_idx == 0 || object_idx > pool->total_count) {
		return NULL;
	}

	return ((char *)object_pool_get_block(pool)) + ((object_idx - 1) * pool->structure_size);
}

void *object_pool_get_data(void *structure) {
	if (structure == NULL) {
		return NULL;
	}

	return ((char *)structure) + MEMORY_FOR_STRUCTURE_HEADERS;
}

object_pool_handle_t object_pool_create(object_pool_count_t object_count, object_pool_size_t object_size) {
	struct object_pool_t *result;
	size_t structure_size;

	if (object_pool_create_will_overflow(object_count, object_size, &structure_size)) {
		return 0;
	}

	result = (struct object_pool_t *)malloc(sizeof(struct object_pool_t) + (object_count * structure_size));
	if (result == NULL) {
		return 0;
	}

	result->next = NULL;
	result->prev = NULL;

	result->head = 0;

	result->free_count = 0;
	result->active_count = 0;
	result->total_count = object_count;

	result->structure_size = structure_size;
	result->object_size = object_size;

	if (!object_pool_global_list_add(result) ||
		!object_pool_create_initialize_objects(result)) {
		free(result);
		return 0;
	}

	return result->handle;
}
int object_pool_destroy(object_pool_handle_t handle) {
	struct object_pool_t *pool;

	pool = object_pool_from_handle(handle);
	if (pool == NULL) {
		return 0;
	}

	if (last_pool == pool) {
		last_pool = NULL;
	}

	free(pool);

	object_pool_global_list_remove(handle);

	return 1;
}

const void *object_pool_from_handle(object_pool_handle_t handle) {
	if (handle == 0 || handle > _object_pool_vector.capacity) {
		return NULL;
	}

	if (last_pool == NULL || last_pool->handle != handle) {
		last_pool = *(struct object_pool_t **)(((char *)_object_pool_vector.data) + ((handle - 1) * sizeof(void *)));
	}

	return last_pool;
}

void *object_pool_ptr_pop(void *pool_ptr) {
	void *result;
	object_pool_index_t *next_idx_p;
	object_pool_index_t self_idx;

	if (pool_ptr == NULL) {
		return NULL;
	}

	result = object_pool_get_from_index(pool_ptr, ((struct object_pool_t *)pool_ptr)->head);
	if (result == NULL) {
		return NULL;
	}
	self_idx = ((struct object_pool_t *)pool_ptr)->head;

	next_idx_p = (object_pool_index_t *)(((char *)result) + sizeof(object_pool_handle_t));

	((struct object_pool_t *)pool_ptr)->head = *next_idx_p;
	*next_idx_p = self_idx;

	((struct object_pool_t *)pool_ptr)->free_count--;
	((struct object_pool_t *)pool_ptr)->active_count++;

	return object_pool_get_data(result);
}

int object_pool_ptr_push(void *pool_ptr, void *object) {
	void *object_structure;
	object_pool_index_t *next_idx_p;
	object_pool_index_t next_idx;

	if (pool_ptr == NULL) {
		return 0;
	}

	if (object == NULL) {
		return 0;
	}

	next_idx_p = (object_pool_index_t *)((((char *)object) - MEMORY_FOR_STRUCTURE_HEADERS) + sizeof(object_pool_handle_t));
	next_idx = ((struct object_pool_t *)(pool_ptr))->head;

	((struct object_pool_t *)pool_ptr)->head = *next_idx_p;

	*next_idx_p = next_idx;

	((struct object_pool_t *)pool_ptr)->free_count++;
	((struct object_pool_t *)pool_ptr)->active_count--;

	return 1;
}

void *object_pool_pop(object_pool_handle_t handle) {
	struct object_pool_t *pool;
	void *result;
	object_pool_index_t *next_idx_p;
	object_pool_index_t self_idx;

	pool = object_pool_from_handle(handle);
	if (pool == NULL || pool->free_count == 0) {
		return NULL;
	}

	result = object_pool_get_from_index(pool, pool->head);
	if (result == NULL) {
		return NULL;
	}
	self_idx = pool->head;

	next_idx_p = (object_pool_index_t *)(((char *)result) + sizeof(object_pool_handle_t));

	pool->head = *next_idx_p;
	*next_idx_p = self_idx;

	pool->free_count--;
	pool->active_count++;

	return object_pool_get_data(result);
}

int object_pool_push(void *object) {
	void *object_structure;
	struct object_pool_t *pool;
	object_pool_index_t *next_idx_p;
	object_pool_index_t next_idx;

	if (object == NULL) {
		return 0;
	}

	object_structure = ((char *)object) - MEMORY_FOR_STRUCTURE_HEADERS;

	if ((pool = object_pool_from_handle(*(object_pool_handle_t *)(object_structure))) == NULL) {
		return 0;
	}

	next_idx_p = (object_pool_index_t *)(((char *)object_structure) + sizeof(object_pool_handle_t));
	next_idx = pool->head;

	pool->head = *next_idx_p;

	*next_idx_p = next_idx;

	pool->free_count++;
	pool->active_count--;

	return 1;
}

object_pool_handle_t object_pool_owns(const void *object) {
	if (object == NULL) {
		return 0;
	}

	return *(object_pool_handle_t *)(((char *)object) - MEMORY_FOR_STRUCTURE_HEADERS);
}

object_pool_count_t object_pool_free_count(object_pool_handle_t handle) {
	struct object_pool_t *pool;
	pool = object_pool_from_handle(handle);
	return pool == NULL ? 0 : pool->free_count;
}

object_pool_count_t object_pool_active_count(object_pool_handle_t handle) {
	struct object_pool_t *pool;
	pool = object_pool_from_handle(handle);
	return pool == NULL ? 0 : pool->active_count;
}

object_pool_count_t object_pool_total_count(object_pool_handle_t handle) {
	struct object_pool_t *pool;
	pool = object_pool_from_handle(handle);
	return pool == NULL ? 0 : pool->total_count;
}

object_pool_size_t object_pool_object_size(object_pool_handle_t handle) {
	struct object_pool_t *pool;
	pool = object_pool_from_handle(handle);
	return pool == NULL ? 0 : pool->object_size;
}

object_pool_size_t object_pool_overhead_size() {
	return MEMORY_FOR_STRUCTURE_HEADERS;
}
