#include "main.h"

int main(int argc, char** argv)
{
    debug(DEBUG_INFO, "Hello World! %s\n", "test");

    for (size_t i = 0; i < MAX_ALLOCATIONS/2; ++i) {
        void* ptr = safe_alloc(1);
        void* ptr2 = safe_alloc(128);
        safe_free((void**) &ptr2);
    }

    // Cleanup
    if (track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_SUCCESS);
    }
    return (EXIT_FAILURE);
}
