#include <gtk/gtk.h>

#include "util.h"

#define NUMBER_OF_FORMS 2
struct GtkEntries {
    GtkWidget* entry[NUMBER_OF_FORMS];
};

// External functions
size_t draw_gui(void);

void draw_main_screen(const char*);
void draw_csv_sync_invite(void);
void draw_csv_sync_screen(GtkWidget*, gpointer);
void fetch_entries(GtkEntry*, gpointer);
void exit_gui(void);

// Definition
#include "gui.c"
