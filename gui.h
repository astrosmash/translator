#include <gtk/gtk.h>

#include "util.h"

#define NUMBER_OF_FORMS 2
struct GtkEntries {
    GtkWidget* entry[NUMBER_OF_FORMS];
};
struct redraw_vbox_arg {
    GtkWidget* box;
    translation_t* translation;
    char* database_file;
};

// External functions
size_t draw_gui(void);

// Definition
#include "gui.c"
