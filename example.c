#include <stdio.h>
#include <string.h>

#include "pool.h"

/* An example structure */
typedef struct packet_t {
	char payload[2048];
	size_t length;
} packet;

void packet_send(packet *pkt) {
	/* Placeholder code for an example function */
	printf("%.*s\n", pkt->length, pkt->payload);
}

int main(int argc, char *argv[]) {

	object_pool pool;  /* The pool structure */
	object_pool_init(&pool); /* First initialize the pool structure before anything else */

	/* Allocate 1024 objects with the size of the packet structure and further initialize the pool structure */
	/* The second argument is the number of objects that will get allocated */
	/* The third argument is the size of an object in bytes */
	object_pool_create(&pool, 1024, sizeof(packet)); 

	/* Pop a packet from the pool */
	/* The pointer returned is owned by the pool structure and handled internally */
	packet *packet_p = object_pool_pop(&pool); 
	if (packet_p != NULL) {

		/* Example packet data */
		const char *packet_payload = "Hello, World!";
		packet_p->length = strlen(packet_payload);
		memcpy(packet_p->payload, packet_payload, packet_p->length);

		packet_send(packet_p); /* Send the packet */

		/* Pushes the packet back to the object pool */
		object_pool_push(packet_p); 
		/* The packet pointer is no longer valid */
		/* The packet structure is now pop'able and back into the cycle */
	}
	else {
		/* The pool has no free objects or error */
		printf("No free objects or error.\n");
		/* Further error handling... */
	}

	object_pool_free(&pool); /* Free the objects and reset the pool */

	return 0;
}
