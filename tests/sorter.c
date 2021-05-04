#define DEBUG_LEVEL DEBUG_TEST

#include "../main.h"

// qsort() + track_block_sort() as comparator

void track_block_sort_test(void)
{
    size_t blocks_len = 16;

    void* unordered_blocks[16] = {
        NULL,
        (void*) 0x7fecc84128b2,
        (void*) 0x7fecc841288a,
        NULL,
        (void*) 0x7fecc8412812,
        NULL,
        (void*) 0x7fecc841289a,
        (void*) 0x7fecc84128a2,
        NULL,
        (void*) 0x7fecc87128aa,
        NULL,
        NULL,
        (void*) 0x7fecc84128dd,
        (void*) 0x7fecc84128ce,
        NULL,
        (void*) 0x7fecc84118cd
    };
    void* ordered_blocks[16] = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        (void*) 0x7fecc84118cd,
        (void*) 0x7fecc8412812,
        (void*) 0x7fecc841288a,
        (void*) 0x7fecc841289a,
        (void*) 0x7fecc84128a2,
        (void*) 0x7fecc84128b2,
        (void*) 0x7fecc84128ce,
        (void*) 0x7fecc84128dd,
        (void*) 0x7fecc87128aa,
    };

    print_table_content((void **) &unordered_blocks, blocks_len - 1);
    qsort(unordered_blocks, blocks_len, sizeof (void *), track_block_sort);
    print_table_content((void **) &unordered_blocks, blocks_len - 1);

    for (size_t i = 0; i < blocks_len; ++i) {
        assert(unordered_blocks[i] == ordered_blocks[i]);
    }
}

int main(int argc, char** argv)
{
    debug(DEBUG_INFO, "Sorter for null blocks test starting... %c", '\n');

    track_block_sort_test();

    // Cleanup
    debug(DEBUG_TEST, "Exiting, track_block should not indicate any leftovers now... %c", '\n');
    if (track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_SUCCESS);
    }
    debug(DEBUG_ERROR, "track_block failed! %c", '\0');
    return (EXIT_FAILURE);
}