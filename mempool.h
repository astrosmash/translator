#include <string.h>

// Do not perform more allocations than specified.
// Useful to prevent allocation loops
#define MAX_ALLOCATIONS 8192

typedef struct {
    // Starting pointer of the array of allocated blocks
    void* allocated_blocks[MAX_ALLOCATIONS];

    // Cached block that's available immediately to omit scanning the pool
    // Useful for a cycle of small frequent allocations/frees
    void* cached_block;

    // Head of the allocated blocks array
    size_t head;

    // Tail of the allocated blocks array
    size_t tail;

    // Number of blocks currently allocated
    size_t count;
} mempool_t;

typedef enum {
    MODE_ALLOCATION = 1 << 0,
    MODE_REMOVAL = 1 << 1,
    MODE_GLOBAL_CLEANUP_ON_SHUTDOWN = 1 << 2
} track_block_modes_t;

// External functions
bool track_block(void*, size_t);
void* safe_alloc(size_t);
void safe_free(void**);

// Definition
#include "mempool.c"
