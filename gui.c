size_t draw_gui(threadpool_t* pool)
{
    assert(pool);
    size_t res = EXIT_SUCCESS;

    debug(DEBUG_INFO, "will use threadpool at %p", (void*)pool);


    // Start initialization
    GtkWidget* main_window = NULL;

    // Main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(main_window);
    gtk_widget_set_name(main_window, "Translator - IELTS helper");

    gtk_window_set_default_size(GTK_WINDOW(main_window), 1600, 800);
    gtk_window_set_title(GTK_WINDOW(main_window), "translator");

    gtk_container_set_border_width(GTK_CONTAINER(main_window), 15);
    g_signal_connect_swapped(main_window, "delete_event", G_CALLBACK(exit_gui), pool);

    gtk_window_present(GTK_WINDOW(main_window));


    // Let's check if a dabatase file exists.
    size_t mode = NEED_TO_CHECK;
    const char* file = db_file(mode);

    gtk_widget_show_all(main_window);


    return res;
}

void exit_gui(gpointer data, GtkWidget* widget)
{
    assert(data);
    threadpool_t* pool = data;
    debug(DEBUG_INFO, "exiting, will stop threadpool at %p", (void*)pool);

    // Shutdown the thread pool
    size_t flags = threadpool_graceful;
    ssize_t res = threadpool_destroy(pool, flags);
    if (res != EXIT_SUCCESS) {
        debug(DEBUG_ERROR, "threadpool_destroy failed: %zi", res);
    }

    gtk_main_quit();
}


const char* get_homedir(void)
{
    const char* homedir = NULL;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    assert(homedir);
    debug(3, "Determined ${HOME} to be %s\n", homedir);

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
    debug(3, "Determined app directory to be %s\n", fullpath);

    size_t res = 0;
    struct stat stat_buf = {0};
    if ((res = stat(fullpath, &stat_buf))) {
        debug(4, "Cannot access app directory %s (%s)\n", fullpath, strerror(errno));

        if (mode & NEED_TO_CREATE) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFDIR) {
                debug(4, "%s is not a directory, will try to create...\n", fullpath);
            }

            if ((res = mkdir(fullpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) {
                debug(1, "Cannot create app directory %s (%s)\n", fullpath, strerror(errno));
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
    debug(3, "Determined database file to be %s\n", db_file);

    if ((res = stat(fullpath, &stat_buf))) {
        debug(4, "Cannot access database file %s (%s)\n", fullpath, strerror(errno));

        if (mode & NEED_TO_CREATE) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFREG) {
                debug(4, "%s is not a file, will try to create...\n", fullpath);
            }

            if ((res = open(fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1) {
                debug(1, "Cannot create database file %s (%s)\n", fullpath, strerror(errno));
                safe_free((void**) &fullpath);
                return NULL;
            }
        } else {
            // Cannot access a file and need_to_create = false
            debug(1, "Cannot access a file and need_to_create = false %s\n", fullpath);
            safe_free((void**) &fullpath);
            return NULL;
        }
    }

    if (mode & NEED_TO_DELETE) {
        if (unlink(fullpath)) {
            debug(1, "Was not able to remove %s (%s)\n", fullpath, strerror(errno));
        }
    }

    // to be freed by caller.
    return fullpath;
}
