# Object Pool (C)
## **A lightweight C89-compatible fixed-size object pool.**

# Overview

This library implements a fixed-size object pool that preallocates memory for a set number of objects and reuses them through a simple API.

# Core Concept

### acquire → take an object from the pool
### use it
### release → return it to the pool

# Example
**A full working example:**
```c
#include <stdio.h>
#include <string.h>

#include "pool.h"

typedef struct packet {
	char payload[2048];
	int length;
} packet_t;

void packet_send(packet_t *pkt)
{
	printf("%.*s\n", pkt->length, pkt->payload);
}

int main(void)
{
	object_pool_t pool;
	if (object_pool_init(&pool, 1024, sizeof(packet_t)) < 0) {
		printf("Object pool initialization failed.\n");
		return -1;
	}

	packet_t *packet_p = object_pool_acquire(&pool);
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
```

# Definitions
**To use custom allocators use OBJECT_POOL_MALLOC and OBJECT_POOL_FREE.**

## Potential Use Cases
- Game engines (entities, particles, bullets)
- Networking (packet reuse)
- Embedded systems
- High-frequency allocation workloads
- Real-time systems

# License
**MIT**
