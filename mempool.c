// Allocation tracker.

static bool track_block(size_t mode)
{
    static mempool_t* pool = NULL;
    if (!pool) {
        pool = malloc(sizof(mempool_t));
        assert(pool);

        memset(pool, 0, sizof(mempool_t));
    }

    if (mode & MODE_ALLOCATION) {
    }

    if (mode & MODE_REMOVAL) {
    }

    if (mode & MODE_GLOBAL_CLEANUP_ON_SHUTDOWN) {
    }

}

// Allocate the block and initialize its contents to zero.
// Argument: size of a block to allocate

void* safe_alloc(size_t size)
{
    void* block = malloc(size);
    assert(block);

    memset(block, 0, size);
    if (track_block(block, MODE_ALLOCATION)) {
        debug(DEBUG_VERBOSE, "Allocated block on %p of size %zu", block, size);
        return block;
    }

    return NULL;
}

// Free the block and set dangling pointer to NULL
// so further checks prevent its usage after free.
// Argument: pointer to a pointer to the allocated block

void safe_free(void** ptr)
{
    assert(*ptr);

    free(*ptr);
    *ptr = NULL;

    debug(DEBUG_VERBOSE, "Freed block on %p", *ptr);
    track_block(*ptr, MODE_REMOVAL);
}
