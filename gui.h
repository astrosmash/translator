#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>

#include <gtk/gtk.h>

enum {
    NEED_TO_CHECK = 1 << 0,
    NEED_TO_CREATE = 1 << 1,
    NEED_TO_DELETE = 1 << 2
} db_file_modes;

// External functions
size_t draw_gui(threadpool_t*);

void exit_gui(gpointer, GtkWidget*);
const char* get_homedir(void);
const char* db_file(size_t);

// Definition
#include "gui.c"
