size_t draw_gui(void)
{
    size_t res = EXIT_SUCCESS;

    bool need_to_allocate = true;
    threadpool_t* pool = get_threadpool(need_to_allocate);
    debug(DEBUG_INFO, "will use threadpool at %p", (void*) pool);

    // Main window
    GtkWidget* main_window = get_main_window(need_to_allocate);
    gtk_widget_set_name(main_window, "Translator - IELTS helper");

    gtk_window_set_default_size(GTK_WINDOW(main_window), 1600, 800);
    gtk_window_set_title(GTK_WINDOW(main_window), "translator");

    gtk_container_set_border_width(GTK_CONTAINER(main_window), 15);
    g_signal_connect_swapped(main_window, "delete_event", G_CALLBACK(exit_gui), NULL);

    gtk_window_present(GTK_WINDOW(main_window));

    // Let's check if a dabatase file exists.
    size_t mode = NEED_TO_CHECK;
    const char* database_file = NULL;

    if ((database_file = db_file(mode)) == NULL) {
        draw_csv_sync_invite();
    } else {
        draw_main_screen(database_file);
    }

    gtk_widget_show_all(main_window);


    return res;
}

void draw_main_screen(const char* database_file)
{
    assert(database_file);
    safe_free((void**) &database_file);
}

void draw_csv_sync_invite(void)
{
    bool need_to_allocate = false;
    GtkWidget* main_window = get_main_window(need_to_allocate);

    GtkWidget *box = NULL, *button = NULL, *grid = NULL, *label = NULL;

    // Main widget
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    assert(box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 5);
    gtk_container_add(GTK_CONTAINER(main_window), box);
    gtk_widget_set_name(box, "main_box");

    // Grid
    grid = gtk_grid_new();
    assert(grid);
    gtk_container_add(GTK_CONTAINER(box), grid);
    gtk_widget_set_name(grid, "main_grid");

    label = gtk_label_new("No local database found. Do you want to fetch Google Spreadsheet?");
    assert(label);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_widget_set_name(label, "no_database_warning");

    button = gtk_button_new_with_label("Yes");
    assert(button);
    g_signal_connect(button, "clicked", G_CALLBACK(draw_csv_sync_screen), box);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);
    gtk_widget_set_name(button, "yes_button");

    button = gtk_button_new_with_label("Quit");
    assert(button);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(exit_gui), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);
    gtk_widget_set_name(button, "quit_button");
}

void draw_csv_sync_screen(GtkWidget* widget, gpointer data)
{
    assert(data);
    GtkWidget *box = data, *grid = NULL, *label = NULL, *spreadsheet_key_entry = NULL, *spreadsheet_gid_entry = NULL;

    // For the program lifetime - we do not want to add form on each click
    static bool pressed = false;

    if (!pressed) {
        // Grid
        grid = gtk_grid_new();
        assert(grid);
        gtk_container_add(GTK_CONTAINER(box), grid);
        gtk_widget_set_name(grid, "main_grid");

        label = gtk_label_new("Spreadsheet Key");
        assert(label);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
        gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
        gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
        gtk_widget_set_name(label, "spreadsheet_key_label");

        spreadsheet_key_entry = gtk_entry_new();
        assert(spreadsheet_key_entry);
        gtk_grid_attach(GTK_GRID(grid), spreadsheet_key_entry, 0, 4, 1, 1);
        gtk_widget_set_name(spreadsheet_key_entry, "spreadsheet_key_entry");

        label = gtk_label_new("Spreadsheet GID");
        assert(label);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
        gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
        gtk_grid_attach(GTK_GRID(grid), label, 0, 5, 1, 1);
        gtk_widget_set_name(label, "spreadsheet_gid_label");

        spreadsheet_gid_entry = gtk_entry_new();
        assert(spreadsheet_gid_entry);
        gtk_grid_attach(GTK_GRID(grid), spreadsheet_gid_entry, 0, 6, 1, 1);
        gtk_widget_set_name(spreadsheet_gid_entry, "spreadsheet_gid_entry");

        bool need_to_allocate = true;
        get_spreadsheet(need_to_allocate); // to be freed by fetch_entries()
        struct GtkEntries* entries = safe_alloc(sizeof (struct GtkEntries)); // to be freed by fetch_entries()
        entries->entry[0] = spreadsheet_key_entry;
        entries->entry[1] = spreadsheet_gid_entry;

        g_signal_connect(GTK_ENTRY(spreadsheet_key_entry), "activate", G_CALLBACK(fetch_entries), entries);
        g_signal_connect(GTK_ENTRY(spreadsheet_gid_entry), "activate", G_CALLBACK(fetch_entries), entries);

        pressed = true;

        // redraw
        gtk_widget_show_all(box);
    }
}

void fetch_entries(GtkEntry* entry, gpointer data)
{
    assert(entry);
    assert(data);
    struct GtkEntries* entries = data;

    bool need_to_allocate = false;
    spreadsheet_t* spreadsheet = get_spreadsheet(need_to_allocate);

    for (size_t i = 0; i < NUMBER_OF_FORMS; i++) {
        assert(entries->entry[i]);

        const char* text = gtk_entry_get_text(GTK_ENTRY(entries->entry[i]));
        if (text) {
            const char* type = gtk_widget_get_name(GTK_WIDGET(entries->entry[i]));
            assert(type);

            size_t text_len = strlen(text);
            assert(text_len < MAX_ENTRY_LENGTH);

            if (text_len) {
                if (strcmp(type, "spreadsheet_key_entry") == 0) {
                    debug(DEBUG_INFO, "Read Spreadsheet Key %s\n", text);
                    if (strlen(spreadsheet->key)) {
                        memset(spreadsheet->key, 0, MAX_ENTRY_LENGTH);
                    }
                    strncpy(spreadsheet->key, text, text_len);

                } else if (strcmp(type, "spreadsheet_gid_entry") == 0) {
                    debug(DEBUG_INFO, "Read Spreadsheet GID %s\n", text);
                    if (strlen(spreadsheet->gid)) {
                        memset(spreadsheet->gid, 0, MAX_ENTRY_LENGTH);
                    }
                    strncpy(spreadsheet->gid, text, text_len);
                } else {
                    debug(DEBUG_ERROR, "text_type unknown %s, doing nothing\n", type);
                }
            }
        }
    }

    if (strlen(spreadsheet->key) && strlen(spreadsheet->gid)) {
        debug(DEBUG_FULLDBG, "Spreadsheet Key length %zu, Spreadsheet GID length %zu\n", strlen(spreadsheet->key), strlen(spreadsheet->gid));

        size_t mode = NEED_TO_CREATE;
        const char* database_file = NULL;

        if ((database_file = db_file(mode)) == NULL) {
            safe_free((void**) &spreadsheet);
            safe_free((void**) &entries);
            return;
        }

//        if (!populate_database(spreadsheet)) {
//            safe_free((void**) &spreadsheet);
//            safe_free((void**) &entries);
//            return;
//        }

        safe_free((void**) &spreadsheet);
        safe_free((void**) &entries);
        draw_main_screen(database_file);

        need_to_allocate = false;
        GtkWidget* main_window = get_main_window(need_to_allocate);
        gtk_widget_show_all(main_window);
    }
}

void exit_gui(void)
{
    bool need_to_allocate = false;
    threadpool_t* pool = get_threadpool(need_to_allocate);

    debug(DEBUG_INFO, "exiting, will stop threadpool at %p", (void*) pool);

    // Shutdown the thread pool
    size_t flags = threadpool_graceful;
    ssize_t res = threadpool_destroy(pool, flags);
    if (res != EXIT_SUCCESS) {
        debug(DEBUG_ERROR, "threadpool_destroy failed: %zi", res);
    }

    gtk_main_quit();
}


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

    debug(DEBUG_FULLDBG, "Returning %p, allocated = %u\n", (void*) threadpool, need_to_allocate);
    return threadpool;
}

GtkWidget* get_main_window(bool need_to_allocate)
{
    static GtkWidget* main_window = NULL;

    if (need_to_allocate) {
        main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        assert(main_window);
    }

    debug(DEBUG_FULLDBG, "Returning %p, allocated = %u\n", (void*) main_window, need_to_allocate);
    return main_window;
}

spreadsheet_t* get_spreadsheet(bool need_to_allocate)
{
    static spreadsheet_t* spreadsheet = NULL;

    if (need_to_allocate) {
        spreadsheet = safe_alloc(sizeof (spreadsheet_t));
    }

    debug(DEBUG_FULLDBG, "Returning %p, allocated = %u\n", (void*) spreadsheet, need_to_allocate);
    return spreadsheet;
}

const char* get_homedir(void)
{
    const char* homedir = NULL;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    assert(homedir);
    debug(DEBUG_FULLDBG, "Determined ${HOME} to be %s\n", homedir);

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
    debug(DEBUG_FULLDBG, "Determined app directory to be %s\n", fullpath);

    size_t res = 0;
    struct stat stat_buf = {0};
    if ((res = stat(fullpath, &stat_buf))) {
        debug(DEBUG_WARN, "Cannot access app directory %s (%s)\n", fullpath, strerror(errno));

        if (mode & NEED_TO_CREATE) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFDIR) {
                debug(DEBUG_WARN, "%s is not a directory, will try to create...\n", fullpath);
            }

            if ((res = mkdir(fullpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) {
                debug(DEBUG_ERROR, "Cannot create app directory %s (%s)\n", fullpath, strerror(errno));
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
    debug(DEBUG_FULLDBG, "Determined database file to be %s\n", db_file);

    if ((res = stat(fullpath, &stat_buf))) {
        debug(DEBUG_WARN, "Cannot access database file %s (%s)\n", fullpath, strerror(errno));

        if (mode & NEED_TO_CREATE) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFREG) {
                debug(DEBUG_WARN, "%s is not a file, will try to create...\n", fullpath);
            }

            if ((res = open(fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1) {
                debug(DEBUG_ERROR, "Cannot create database file %s (%s)\n", fullpath, strerror(errno));
                safe_free((void**) &fullpath);
                return NULL;
            }
        } else {
            // Cannot access a file and need_to_create = false
            debug(DEBUG_ERROR, "Cannot access a file and need_to_create = false %s\n", fullpath);
            safe_free((void**) &fullpath);
            return NULL;
        }
    }

    if (mode & NEED_TO_DELETE) {
        if (unlink(fullpath)) {
            debug(DEBUG_ERROR, "Was not able to remove %s (%s)\n", fullpath, strerror(errno));
        }
    }

    // to be freed by caller.
    return fullpath;
}
