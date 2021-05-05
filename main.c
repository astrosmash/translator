#include "main.h"

int main(int argc, char** argv)
{
    ssize_t opt = 0;
    size_t gui = 0;

    while ((opt = getopt(argc, argv, "g")) != -1) {
        switch (opt) {
        case 'g':
            gui = 1;
            break;
        default: /* '?' */
            debug(DEBUG_ERROR, "Usage: %s [-g]\n", *argv);
            return (EXIT_FAILURE);
        }
    }

    if (gui) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        gtk_init(&argc, &argv);

        // Launch the thread pool
        size_t thread_count = MAX_THREADS - 1;
        size_t queue_size = MAX_QUEUE / 2;
        size_t flags = 0;
        threadpool_t* pool = threadpool_create(thread_count, queue_size, flags);

        if (draw_gui(pool)) {
            debug(DEBUG_ERROR, "Cannot launch GUI %c", '\n');
        }

        gtk_main();
        curl_global_cleanup();
    }

    // Cleanup
    if (track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_SUCCESS);
    }
    return (EXIT_FAILURE);
}
