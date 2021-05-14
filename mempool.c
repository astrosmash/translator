// Helper function for quick sort to keep NULLs on top
int track_block_sort(const void* ptr1, const void* ptr2)
{
    const void* arg1 = *(const void **) ptr1;
    const void* arg2 = *(const void **) ptr2;
    debug_test("arg1: %p arg2: %p\n", arg1, arg2);

    // !arg1 && !arg2 falls there
    if (arg1 == arg2) {
        return 0;
    }

    if (!arg1) return 1;
    if (!arg2) return -1;

    return (arg1 - arg2);
}

// Helper function to print block table.
void print_table_content(void** start, size_t num)
{
    assert(start);
    for (size_t i = 0; i <= num; ++i) {
        debug_test("allocated_blocks#%zu %p", i, *(start + i));
    }
    debug_test("%c", '\n');
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

    if (mode & MODE_ALLOCATION) {
        assert(ptr);

        // Use cached_block if it's free so we don't lookup the array
        if (!pool->cached_block) {
            debug_fulldbg("Will insert %p at cached_block", ptr);
            pool->cached_block = ptr;
            return true;
        }

        ++pool->count;
        assert(pool->count < MAX_ALLOCATIONS);

        if (!(pool->allocated_blocks[pool->count])) {
            pool->allocated_blocks[pool->count] = ptr;
        } else {
            debug_error("Failed to insert %p at pos %zu - found %p there", ptr, pool->count, pool->allocated_blocks[pool->count]);

#ifdef DEBUG
            print_table_content((void **) &(pool->allocated_blocks), pool->count);
#endif
            // Try sorting and try again
            qsort(pool, pool->count + 1, sizeof (void *), track_block_sort);
#ifdef DEBUG
            print_table_content((void **) &(pool->allocated_blocks), pool->count);
#endif

            if (!(pool->allocated_blocks[pool->count])) {
                pool->allocated_blocks[pool->count] = ptr;
            } else {
                debug_error("Failed to insert %p at pos %zu - found %p there, sorting did not help! "
                        "Will increment allocated counter so we have null on the top",
                        ptr,
                        pool->count,
                        pool->allocated_blocks[pool->count]);

                // Try to increment allocated counter as last resort...
                ++pool->count;

                if (!(pool->allocated_blocks[pool->count])) {
                    pool->allocated_blocks[pool->count] = ptr;
                } else {
                    // Techincally should never happen
                    debug_error("Incrementing did not help - have %p there...", pool->allocated_blocks[pool->count]);
                    return false;
                }
            }
        }
        debug_verbose("Inserted %p at pos %zu", ptr, pool->count);
        return true;
    }


    if (mode & MODE_REMOVAL) {
        assert(ptr);

        // Free cached_block so it's available for next allocation
        if (pool->cached_block == ptr) {
            debug_fulldbg("Found %p at cached_block", ptr);
            pool->cached_block = NULL;
            return true;
        }

        for (size_t i = pool->count; i > 0; --i) {
            debug_fulldbg("Looking for %p, found %p at pos %zu", ptr, pool->allocated_blocks[i], i);
            if (pool->allocated_blocks[i] == ptr) {
                pool->allocated_blocks[i] = NULL;

                // We decrement a number of allocated blocks, but if our NULLed block was not on top, top block will get lost.
                // Copy previous block to our new NULLed position so it could be tracked later
                if (pool->allocated_blocks[i + 1]) {
                    pool->allocated_blocks[i] = pool->allocated_blocks[i + 1];
                    pool->allocated_blocks[i + 1] = NULL;
                }

                --pool->count;
                debug_verbose("Removed %p at pos %zu, now allocated: %zu", ptr, i, pool->count);
                return true;
            }
        }
        if (pool->allocated_blocks[0] == ptr) {
            pool->allocated_blocks[0] = NULL;
            return true;
        }
        debug_error("Failed to find %p for removal...", ptr);
        return false;
    }


    if (mode & MODE_GLOBAL_CLEANUP_ON_SHUTDOWN) {
        for (size_t i = pool->count; i > 0; --i) {
            if (pool->allocated_blocks[i]) {
                debug_fulldbg("Removing leftover %p at pos %zu", pool->allocated_blocks[i], i);
                free(pool->allocated_blocks[i]);
                pool->allocated_blocks[i] = NULL;
                --pool->count;
            }
        }
        if (pool->count) {
            debug_verbose("%zu blocks were already NULL", pool->count);
        }

        // We are exiting, free the pool
        free(pool);
        pool = NULL;
        return true;
    }
    debug_error("%zu blocks MISSED in the pool...", pool->count);

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
        debug_verbose("Allocated block on %p of size %zu", block, size);
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
    debug_verbose("Freed block on %p", *ptr);
    track_block(*ptr, MODE_REMOVAL);

    free(*ptr);
    *ptr = NULL;
}
