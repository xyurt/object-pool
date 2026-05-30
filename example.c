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

int main(int argc, char *argv[]) {
 
	object_pool pool = object_pool_create(1024, sizeof(packet));

	packet *packet_p = object_pool_pop(pool);
	if (packet_p != NULL) {
		const char *packet_payload = "Hello, World!";
		packet_p->length = strlen(packet_payload);
		memcpy(packet_p->payload, packet_payload, packet_p->length);

		packet_send(packet_p);

		/* Pointer becomes invalid after this call */
		object_pool_push(packet_p);
	}
	else {
		printf("Pool exhausted or allocation failure.\n");
	}

/* Handle becomes invalid after this call */
	object_pool_destroy(pool);

	return 0;
}
