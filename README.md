# Object Pool (C)

## C89-compliant low-overhead fixed-size object pool for C, designed for systems and performance-critical software, providing a simple API with deterministic O(1) allocation and reuse of preallocated objects, reducing heap fragmentation.

# Overview

This library implements a fixed-size object pool that preallocates memory for a set number of objects and reuses them through a simple push/pop API.

The goal is predictable performance and minimal allocation overhead in long-running or high-frequency allocation environments.

## Features
- C89 compatible
- Fixed-size object pooling
- Simple push/pop API
- O(1) allocation and deallocation
- Reduced heap fragmentation through reuse
- Optional dynamic implementation mode
- Lightweight design for systems and embedded use

# Core Concept
## Instead of repeatedly allocating and freeing memory, a block of objects is allocated once and reused.

###  pop → take an object from the pool
### use it
### push → return it to the pool

## Example
**A full working example is provided in:**
**example.c**

## Potential Use Cases
- Game engines (entities, particles, bullets)
- Networking (packet reuse)
- Embedded systems
- High-frequency allocation workloads
- Real-time systems
- Anything else that comes to mind

# License
# MIT
