#include <stdio.h>
#include <string.h>

#include "pool.h"

/* An example structure */
typedef struct packet_t {
	char payload[2048];
	int length;
} packet;

/* Placeholder code for an example function */
void packet_send(packet *pkt) {
	printf("%.*s\n", pkt->length, pkt->payload);
}

int main(int argc, char *argv[]) {

	/* Create a pool of reusable packet objects */
	object_pool pool = object_pool_create(1024, sizeof(packet));

	packet *packet_p = object_pool_pop(pool);
	if (packet_p != NULL) {
		const char *packet_payload = "Hello, World!";
		packet_p->length = strlen(packet_payload);
		memcpy(packet_p->payload, packet_payload, packet_p->length);

		packet_send(packet_p);

		/* Return object to pool.
		   Pointer becomes invalid after this call */
		object_pool_push(packet_p);
	}
	else {
		printf("Pool exhausted or allocation failure.\n");
	}

	object_pool_destroy(pool);

	return 0;
}
