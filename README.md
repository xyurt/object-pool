# Object Pool (C)

## A simple C89-compatible fixed-size object pool.

# Overview

This library implements a fixed-size object pool that preallocates memory for a set number of objects and reuses them through a simple push/pop API.

The goal is predictable performance and minimal allocation overhead in long-running or high-frequency allocation environments.

## Features
- C89 compatible
- Fixed-size object pooling
- Simple push/pop API
- O(1) allocation and deallocation
- Reduced heap fragmentation through reuse
- Lightweight design for systems and embedded use

# Core Concept
**Instead of repeatedly allocating and freeing memory, a block of objects is allocated once and reused.**

### pop → take an object from the pool
### use it
### push → return it to the pool

# Example
**A full working example of the life cycle is provided in:**
**example.c**

# Function Variants - Performance
Aside from the basic and ergonomic API 
you can also use the function variants:

**object_pool_ptr_pop(pool_ptr) / object_pool_ptr_push(pool_ptr, object)**

They operate with the raw pointer of the object pool
which boosts up the performance significantly

The raw pointer of the object pool can be 
retrieved with **object_pool_from_handle(handle)**

## Potential Use Cases
- Game engines (entities, particles, bullets)
- Networking (packet reuse)
- Embedded systems
- High-frequency allocation workloads
- Real-time systems

# License
**MIT**
