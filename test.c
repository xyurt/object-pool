#include <stdio.h>

#include "pool.h"

#include <string.h>

#define TEST(e) (e) == 0 ? "[SUCCESS]" : "[FAIL]"
#define TEST_VALID(e) (e) != 0 ? "[SUCCESS]" : "[FAIL]"
#define TEST_NULL(e) (e) != NULL ? "[SUCCESS]" : "[FAIL]"
#define TEST_BOOL(e) (e) == 1 ? "[TRUE]" : "[FALSE]"

typedef struct packet_t {
	char payload[2048];
	size_t length;
} packet;

int main(int argc, char *argv[]) {
	object_pool pool;
	printf("[INIT] Initialization: %s\n\n", TEST(object_pool_init(&pool)));

	size_t capacity = 4096;

	printf("[CREATE]: %s\npacket pool: {\n capacity: %zu\n}\n\n", TEST(object_pool_create(&pool, capacity, sizeof(packet))), capacity);

	printf("[TEST] Are there free packets available: %s\n", TEST_BOOL(!object_pool_empty(&pool)));
	printf("[TEST] Capacity of the packet pool: %zu\n", object_pool_capacity(&pool));
	printf("[TEST] The object size of the pool: %zu\n", object_pool_object_size(&pool));
	printf("[TEST] The full object size of the pool: %zu\n", object_pool_object_full_size(&pool));
	printf("[TEST] The memory footprint of the pool: %zu\n\n", object_pool_memory_footprint(&pool));

	size_t magic_number = 0xAABBCCDDEEFF;

#ifndef _OBJECT_POOL_DYNAMIC_IMPL 
	packet *test_packet;
	printf("[TEST] Popping a packet from the pool: %s\n", TEST_NULL(test_packet = object_pool_pop(&pool)));
	printf("[TEST] Pushing the packet back into the pool: %s\n\n", TEST(object_pool_push(test_packet)));

	printf("[TEST] Popping a packet and writing a magic number in: ");
	if ((test_packet = object_pool_pop(&pool)) == NULL) {
		printf("[FAIL]\n");
	}
	else {
		memcpy(test_packet->payload, &magic_number, sizeof(size_t));
		printf("[SUCCESS]\n");
	}

	printf("[TEST] Pushing the packet back into the pool: %s\n", TEST(object_pool_push(test_packet)));
	printf("[TEST] Popping the packet and checking if the magic number corrupted: ");
	if ((test_packet = object_pool_pop(&pool)) == NULL) {
		printf("[FAIL]\n");
	}
	else {
		printf("%s\n", TEST(memcmp(test_packet->payload, &magic_number, sizeof(size_t))));
	}
	printf("[TEST] Pushing the packet back into the pool: %s\n\n", TEST(object_pool_push(test_packet)));
#endif

#ifdef _OBJECT_POOL_DYNAMIC_IMPL
	size_t test_handle;
	printf("[TEST] Popping a packet from the pool: %s\n", TEST_VALID(test_handle = object_pool_pop(&pool)));
	printf("[TEST] Pushing the packet back into the pool using the handle: %s\n\n", TEST(object_pool_push_handle(&pool, test_handle)));

	packet *test_packet;
	printf("[TEST] Popping a packet from the pool: %s\n", TEST_VALID(test_handle = object_pool_pop(&pool)));
	printf("[TEST] Getting the packet pointer: %s\n", TEST_NULL(test_packet = object_pool_get(&pool, test_handle)));
	printf("[TEST] Pushing the packet back into the pool using the pointer: %s\n\n", TEST(object_pool_push(test_packet)));

	printf("[TEST] Popping a packet and writing a magic number in: ");
	if ((test_handle = object_pool_pop(&pool)) == 0 || (test_packet = object_pool_get(&pool, test_handle)) == NULL) {
		printf("[FAIL]\n");
	}
	else {
		memcpy(test_packet->payload, &magic_number, sizeof(size_t));
		printf("[SUCCESS]\n");
	}
	printf("[TEST] Pushing the packet back into the pool: %s\n", TEST(object_pool_push(test_packet)));
	printf("[TEST] Popping the packet and checking if the magic number corrupted: ");
	if ((test_handle = object_pool_pop(&pool)) == 0 || (test_packet = object_pool_get(&pool, test_handle)) == NULL) {
		printf("[FAIL]\n");
	}
	else {
		printf("%s\n", TEST(memcmp(test_packet->payload, &magic_number, sizeof(size_t))));
	}
	printf("[TEST] Pushing the packet back into the pool: %s\n\n", TEST(object_pool_push(test_packet)));
#endif

	printf("[FREE] Freeing: %s\n\n", TEST(object_pool_free(&pool)));

	return 0;
}