# Object Pool (C)
# **A lightweight C89-compatible fixed-size object pool.**

# Overview

This library implements a fixed-size object pool that preallocates memory for a set number of objects and reuses them through a simple API.

# Core Concept

### acquire → take an object from the pool
### use it
### release → return it to the pool

# Example
**A full working example of the life cycle is provided in:**
**example.c**

# Definitions
**To use custom allocators use OBJECT_POOL_MALLOC and OBJECT_POOL_FREE.**
**To changing the index type of the objects use OBJECT_POOL_INDEX_TYPE. Its default value is size_t. Changing this will change the header size of each object.**

## Potential Use Cases
- Game engines (entities, particles, bullets)
- Networking (packet reuse)
- Embedded systems
- High-frequency allocation workloads
- Real-time systems

# License
**MIT**
