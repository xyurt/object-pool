#include <stdio.h>
#include <string.h>

#include "pool.h"

typedef struct packet_t {
	char payload[2048];
	size_t length;
} packet;

void packet_send(packet *pkt) {
	printf("%.*s\n", pkt->length, pkt->payload);
}

int main(int argc, char *argv[]) {

	object_pool pool;
	object_pool_init(&pool);
	object_pool_create(&pool, 1024, sizeof(packet)); 

	packet *packet_p = object_pool_pop(&pool); 
	if (packet_p != NULL) {
		const char *packet_payload = "Hello, World!";
		packet_p->length = strlen(packet_payload);
		memcpy(packet_p->payload, packet_payload, packet_p->length);

		packet_send(packet_p);

		object_pool_push(packet_p); 
	}
	else {
		printf("No free objects or error.\n");
	}

	object_pool_free(&pool);

	return 0;
}