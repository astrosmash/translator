#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>

#define MAX_ENTRY_LENGTH 256
typedef struct {
    char key[MAX_ENTRY_LENGTH];
    char gid[MAX_ENTRY_LENGTH];
} spreadsheet_t;

struct curl_string {
    char* ptr;
    size_t len;
};

enum {
    NEED_TO_CHECK = 1 << 0,
    NEED_TO_CREATE = 1 << 1,
    NEED_TO_DELETE = 1 << 2
} db_file_modes;

threadpool_t* get_threadpool(bool);
GtkWidget* get_main_window(bool);
spreadsheet_t* get_spreadsheet(bool);
const char* db_file(size_t);
bool populate_database(const char*);

// Definition
#include "util.c"
