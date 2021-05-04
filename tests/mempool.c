#define DEBUG_LEVEL DEBUG_TEST

#include "../main.h"

// safe_alloc()
void safe_alloc_free_test(void) {
    uint16_t* block_16 = safe_alloc(16);
    assert(block_16);
    assert(*block_16 == 0);

    memset(block_16, 42, 16);
    assert(*block_16 == 10794);

    void* block_256 = safe_alloc(256);
    assert(block_256);

    char* test_str = "ABCDEFabcdef";
    memcpy(block_256, test_str, strlen(test_str));
    assert(strcmp(block_256, test_str) == 0);
    assert(strcmp(block_256, "fake"));

    char* test_str2 = "DSU2OR3DIJSADKKJDJKJKDSU23OPK";
    memcpy(block_256 + strlen(test_str), test_str2, strlen(test_str2));
    assert(strcmp(block_256, "ABCDEFabcdefDSU2OR3DIJSADKKJDJKJKDSU23OPK") == 0);
    assert(strcmp(block_256, "fake"));


    safe_free((void**) &block_16);
    assert(block_16 == NULL);
    safe_free((void**) &block_256);
    assert(block_256 == NULL);
}

// track_block()
void track_block_test(void)
{
    void* test_block = safe_alloc(16);
    assert(track_block(test_block, MODE_REMOVAL));
    assert(!track_block(test_block, MODE_REMOVAL));

    assert(track_block(test_block, MODE_ALLOCATION));


    for (size_t i = 0; i < MAX_ALLOCATIONS - 1; ++i) {
        assert(track_block(test_block, MODE_ALLOCATION));
    }

    for (size_t i = 1; i < MAX_ALLOCATIONS; ++i) {
        assert(track_block(test_block, MODE_REMOVAL));
    }

    assert(!track_block(test_block, MODE_REMOVAL));


    // Reverse
    for (size_t i = 0; i < MAX_ALLOCATIONS - 1; ++i) {
        assert(track_block(test_block, MODE_ALLOCATION));
    }

    for (size_t i = MAX_ALLOCATIONS; i > 1; --i) {
        assert(track_block(test_block, MODE_REMOVAL));
    }

    assert(!track_block(test_block, MODE_REMOVAL));

    // Should not indicate any leftovers
    assert(track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN));
}


int main(int argc, char** argv)
{
    debug(DEBUG_INFO, "Mempool test starting... %c", '\n');

    safe_alloc_free_test();
    track_block_test();

    // Cleanup
    debug(DEBUG_TEST, "Exiting, track_block should not indicate any leftovers now... %c", '\0');
    if (track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_SUCCESS);
    }
    debug(DEBUG_ERROR, "track_block failed! %c", '\0');
    return (EXIT_FAILURE);
}
