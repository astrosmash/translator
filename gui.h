#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>

#include <gtk/gtk.h>

enum {
    NEED_TO_CHECK = 1 << 0,
    NEED_TO_CREATE = 1 << 1,
    NEED_TO_DELETE = 1 << 2
} db_file_modes;

#define NUMBER_OF_FORMS 2
struct GtkEntries {
    GtkWidget* entry[NUMBER_OF_FORMS];
};

#define MAX_ENTRY_LENGTH 256
typedef struct {
    char key[MAX_ENTRY_LENGTH];
    char gid[MAX_ENTRY_LENGTH];
} spreadsheet_t;

// External functions
size_t draw_gui(void);

void draw_main_screen(const char*);
void draw_csv_sync_invite(void);
void draw_csv_sync_screen(GtkWidget*, gpointer);
void fetch_entries(GtkEntry*, gpointer);
void exit_gui(void);

threadpool_t* get_threadpool(bool);
GtkWidget* get_main_window(bool);
spreadsheet_t* get_spreadsheet(bool);
const char* get_homedir(void);
const char* db_file(size_t);

// Definition
#include "gui.c"
