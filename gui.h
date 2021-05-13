#include <gtk/gtk.h>

#include "util.h"

#define NUMBER_OF_FORMS 2
struct GtkEntries {
    GtkWidget* entry[NUMBER_OF_FORMS];
};

// External functions
size_t draw_gui(void);

// Definition
#include "gui.c"
