// Helper function for quick sort to keep NULLs on top

static int track_block_sort(const void* ptr1, const void* ptr2)
{
    // !ptr1 && !ptr2 falls there
    if (ptr1 == ptr2) {
        return 0;
    }

    if (!ptr1) return -1;
    if (!ptr2) return 1;
}

// Allocation tracker.

bool track_block(void* ptr, size_t mode)
{
    static mempool_t* pool = NULL;
    if (!pool) {
        pool = malloc(sizeof (mempool_t));
        assert(pool);

        memset(pool, 0, sizeof (mempool_t));

        // Initialize the pool.
        for (size_t i = 0; i <= MAX_ALLOCATIONS; ++i) {
            pool->allocated_blocks[i] = NULL;
        }
    }

    qsort(pool, pool->count, sizeof (void *), track_block_sort);


    if (mode & MODE_ALLOCATION) {
        assert(ptr);

        // Use cached_block if it's free so we don't lookup the array
        if (!pool->cached_block) {
            debug(DEBUG_FULLDBG, "Will insert %p at cached_block", ptr);
            pool->cached_block = ptr;
            return true;
        }

        ++pool->count;
        assert(pool->count < MAX_ALLOCATIONS);

        if (!(pool->allocated_blocks[pool->count])) {
            pool->allocated_blocks[pool->count] = ptr;
        } else {
            debug(DEBUG_ERROR, "Failed to insert %p at pos %zu - found %p there", ptr, pool->count, pool->allocated_blocks[pool->count]);
            return false;
        }
        debug(DEBUG_VERBOSE, "Inserted %p at pos %zu", ptr, pool->count);
        return true;
    }


    if (mode & MODE_REMOVAL) {
        assert(ptr);

        // Free cached_block so it's available for next allocation
        if (pool->cached_block == ptr) {
            debug(DEBUG_FULLDBG, "Found %p at cached_block", ptr);
            pool->cached_block = NULL;
            return true;
        }

        for (size_t i = pool->count; i > 0; --i) {
            debug(DEBUG_FULLDBG, "Looking for %p, found %p at pos %zu", ptr, pool->allocated_blocks[i], i);
            if (pool->allocated_blocks[i] == ptr) {
                pool->allocated_blocks[i] = NULL;

                debug(DEBUG_VERBOSE, "Removed %p at pos %zu, now allocated: %zu", ptr, i, pool->count);
                --pool->count;
                return true;
            }
        }
        debug(DEBUG_ERROR, "Failed to find %p for removal...", ptr);
        return false;
    }


    if (mode & MODE_GLOBAL_CLEANUP_ON_SHUTDOWN) {
        for (size_t i = pool->count; i > 0; --i) {
            if (pool->allocated_blocks[i]) {
                debug(DEBUG_FULLDBG, "Removing leftover %p at pos %zu", pool->allocated_blocks[i], i);
                free(pool->allocated_blocks[i]);
                pool->allocated_blocks[i] = NULL;
                --pool->count;
            }
        }
        if (pool->count) {
            debug(DEBUG_VERBOSE, "%zu blocks were already NULL", pool->count);
        }

        // We are exiting, free the pool
        free(pool);
        pool = NULL;
        return true;
    }
    debug(DEBUG_ERROR, "%zu blocks MISSED in the pool...", pool->count);

    // We are exiting, free the pool
    free(pool);
    pool = NULL;
    return false;
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
    debug(DEBUG_VERBOSE, "Freed block on %p", *ptr);
    track_block(*ptr, MODE_REMOVAL);

    free(*ptr);
    *ptr = NULL;
}
