#include <stdio.h>
#include <string.h>

#include "pool.h"

typedef struct packet_t {
	char payload[2048];
	int length;
} packet;

void packet_send(packet *pkt) {
	printf("%.*s\n", pkt->length, pkt->payload);
}

int main(void)
{
	object_pool_t pool;
	if (object_pool_init(&pool, 1024, sizeof(packet)) < 0) {
		printf("Object pool initialization failed.\n");
		return -1;
	}

	packet *packet_p = object_pool_acquire(&pool);
	if (packet_p) {
		const char *packet_payload = "Hello, World!";
		packet_p->length = strlen(packet_payload);
		memcpy(packet_p->payload, packet_payload, packet_p->length);

		packet_send(packet_p);

		object_pool_release(&pool, packet_p);
	}
	else {
		printf("Pool exhausted or allocation failure.\n");
	}

	object_pool_cleanup(&pool);

	return 0;
}
