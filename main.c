#include "main.h"

int main(int argc, char** argv)
{
    debug(DEBUG_INFO, "Hello World! %s\n", "test");

    // Cleanup
    if (track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_SUCCESS);
    }
    return (EXIT_FAILURE);
}
