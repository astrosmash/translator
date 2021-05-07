threadpool_t* get_threadpool(bool need_to_allocate)
{
    static threadpool_t* threadpool = NULL;

    if (need_to_allocate) {
        // Launch the thread pool
        size_t thread_count = MAX_THREADS - 1;
        size_t queue_size = MAX_QUEUE / 2;
        size_t flags = 0;
        threadpool = threadpool_create(thread_count, queue_size, flags);
    }

    debug_fulldbg("Returning %p, allocated = %u\n", (void*) threadpool, need_to_allocate);
    return threadpool;
}

GtkWidget* get_main_window(bool need_to_allocate)
{
    static GtkWidget* main_window = NULL;

    if (need_to_allocate) {
        main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        assert(main_window);
    }

    debug_fulldbg("Returning %p, allocated = %u\n", (void*) main_window, need_to_allocate);
    return main_window;
}

spreadsheet_t* get_spreadsheet(bool need_to_allocate)
{
    static spreadsheet_t* spreadsheet = NULL;

    if (need_to_allocate) {
        spreadsheet = safe_alloc(sizeof (spreadsheet_t));
    }

    debug_fulldbg("Returning %p, allocated = %u\n", (void*) spreadsheet, need_to_allocate);
    return spreadsheet;
}

const char* get_homedir(void)
{
    const char* homedir = NULL;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    assert(homedir);
    debug_fulldbg("Determined ${HOME} to be %s\n", homedir);

    return homedir;
}

const char* db_file(size_t mode)
{
    const char* homedir = get_homedir();
    const char* app_subdir = "/.tiny-ielts";
    const char* db_file = "/.db";

    size_t fullpathsize = strlen(homedir) + strlen(app_subdir) + strlen(db_file) + 1;
    char* fullpath = safe_alloc(fullpathsize);

    strncpy(fullpath, homedir, strlen(homedir));
    strncat(fullpath, app_subdir, strlen(app_subdir));
    debug_fulldbg("Determined app directory to be %s\n", fullpath);

    size_t res = 0;
    struct stat stat_buf = {0};
    if ((res = stat(fullpath, &stat_buf))) {
        debug_warn("Cannot access app directory %s (%s)\n", fullpath, strerror(errno));

        if (mode & NEED_TO_CREATE) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFDIR) {
                debug_warn("%s is not a directory, will try to create...\n", fullpath);
            }

            if ((res = mkdir(fullpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) {
                debug_error("Cannot create app directory %s (%s)\n", fullpath, strerror(errno));
                safe_free((void**) &fullpath);
                return NULL;
            }
        } else {
            // Cannot access a directory and need_to_create = false
            safe_free((void**) &fullpath);
            return NULL;
        }
    }

    strncat(fullpath, db_file, strlen(db_file));
    debug_fulldbg("Determined database file to be %s\n", db_file);

    if ((res = stat(fullpath, &stat_buf))) {
        debug_warn("Cannot access database file %s (%s)\n", fullpath, strerror(errno));

        if (mode & NEED_TO_CREATE) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFREG) {
                debug_warn("%s is not a file, will try to create...\n", fullpath);
            }

            if ((res = open(fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1) {
                debug_error("Cannot create database file %s (%s)\n", fullpath, strerror(errno));
                safe_free((void**) &fullpath);
                return NULL;
            }
        } else {
            // Cannot access a file and need_to_create = false
            debug_error("Cannot access a file and need_to_create = false %s\n", fullpath);
            safe_free((void**) &fullpath);
            return NULL;
        }
    }

    if (mode & NEED_TO_DELETE) {
        if (unlink(fullpath)) {
            debug_error("Was not able to remove %s (%s)\n", fullpath, strerror(errno));
        }
    }

    // to be freed by caller.
    return fullpath;
}
