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
            debug_error("Usage: %s [-g]\n", *argv);
            return (EXIT_FAILURE);
        }
    }

    srand(time(NULL));
    if (gui) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        gtk_init(&argc, &argv);

        if (draw_gui()) {
            debug_error("Cannot launch GUI %c", '\n');
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
