static void draw_popup_menu(GtkWidget* widget, GdkEvent* event)
{
    assert(widget);
    assert(event);

    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton* bevent = (GdkEventButton*) event;
        if (bevent->button == 3) { // Right mouse click
            gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,
                    bevent->button, bevent->time);
        }
    }
}

static void clear_children(GtkWidget* widget)
{
    assert(widget);

    // Remove all widgets that are present on the main window
    if (GTK_IS_CONTAINER(widget)) {
        GList* children = gtk_container_get_children(GTK_CONTAINER(widget));
        for (const GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
            debug_verbose("Clearing child at %p\n", iter->data);
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
        g_list_free(children);
    }
}

static void save_sentence(GtkWidget* widget, GdkEvent* event)
{
    assert(widget);
    assert(event);
    const char* sentence = gtk_entry_get_text(GTK_ENTRY(widget));
    const char* or_word = (const char*) event;
    debug_test("sentence: %s or_word: %s\n", sentence, or_word);

    if (strstr(sentence, " /s") || strstr(sentence, " /с")) {
        // see db_file()
        const char* homedir = get_homedir();
        const char* app_subdir = "/.tiny-ielts";
        const char* db_file = "/.sentencedb";

        size_t fullpathsize = strlen(homedir) + strlen(app_subdir) + strlen(db_file) + 1;
        char* fullpath = safe_alloc(fullpathsize);

        strncpy(fullpath, homedir, strlen(homedir));
        strncat(fullpath, app_subdir, strlen(app_subdir));
        strncat(fullpath, db_file, strlen(db_file));

        DB *db_p = NULL;
        ssize_t db_ret = 0;
        if ((db_ret = db_create(&db_p, NULL, 0))) {
            debug_error("Cannot db_create (%s)\n", db_strerror(db_ret));
            safe_free((void**) &fullpath);
            return;
        }
        assert(db_p);

        size_t db_flags = DB_CREATE;
        size_t db_mode = DB_HASH;
        if ((db_ret = db_p->open(db_p, NULL, fullpath, NULL, db_mode, db_flags, 0600))) {
            debug_error("Cannot db_p->open (%s)\n", db_strerror(db_ret));
            safe_free((void**) &fullpath);
            return;
        }

        DBT key;
        memset(&key, 0, sizeof (key));
        DBT value;
        memset(&value, 0, sizeof (value));

        key.data = (void*) or_word;
        key.size = strlen(or_word);
        value.data = (void*) sentence;
        value.size = strlen(sentence);

        if ((db_ret = db_p->put(db_p, NULL, &key, &value, 0))) {
            debug_error("Cannot db_p->put (%s)\n", db_strerror(db_ret));
            safe_free((void**) &fullpath);
            return;
        }

        if ((db_ret = db_p->get(db_p, NULL, &key, &value, 0))) {
            debug_error("Cannot db_p->get (%s)\n", db_strerror(db_ret));
            safe_free((void**) &fullpath);
            return;
        }

        if ((db_ret = db_p->close(db_p, 0))) {
            debug_error("Cannot db_p->close (%s)\n", db_strerror(db_ret));
            safe_free((void**) &fullpath);
            return;
        }

        safe_free((void**) &fullpath);
    }
}

static void draw_main_screen(const char*);

static void redraw_vbox(GtkWidget* parent, gpointer data)
{
    assert(data);
    struct redraw_vbox_arg* arg = data;

    const char* answer = NULL;
    if (GTK_IS_ENTRY(parent)) {
        answer = gtk_entry_get_text(GTK_ENTRY(parent));
        if (!strlen(answer)) return;
    }

    GtkWidget* vbox = arg->box;
    translation_t* translation = arg->translation;

    if (GTK_IS_CONTAINER(vbox)) {
        GList* vbox_widgets = gtk_container_get_children(GTK_CONTAINER(vbox));

        for (const GList* vbox_iter = vbox_widgets; vbox_iter != NULL; vbox_iter = g_list_next(vbox_iter)) {
            if (GTK_IS_CONTAINER(vbox_iter->data)) {
                GList* eventbox_view = gtk_container_get_children(GTK_CONTAINER(vbox_iter->data));

                for (const GList* eventbox_elements = eventbox_view; eventbox_elements != NULL; eventbox_elements = g_list_next(eventbox_elements)) {
                    const char* current_label_text = gtk_label_get_label(GTK_LABEL(eventbox_elements->data));
                    if (strlen(current_label_text)) {
                        debug_test("current_label_text: %s\n", current_label_text);

                        static size_t wrong_answer_num = 0;
                        if (answer) {
                            if (strcmp(answer, translation->tr_word)) {
                                debug_fulldbg("wrong answer: %s (%s)\n", answer, translation->tr_word);

                                size_t total_len = strlen(current_label_text) + strlen(translation->tr_word);
                                char* newtext = safe_alloc(total_len + 50);
                                // This is a second wrong answer, give a hint
                                if (strstr(current_label_text, "FF0000")) {
                                    char* hint = safe_alloc(strlen(translation->tr_word));

                                    if (wrong_answer_num < 5) {
                                        // Unicode 2 bytes per symbol
                                        for (size_t i = 0; i < 3 * 2; ++i) {
                                            memcpy((hint + i), (translation->tr_word + i), sizeof (*(translation->tr_word + i)));
                                        }
                                        strncat(hint, "...", strlen("..."));
                                        snprintf(newtext, total_len, "<span color='#FF0000'>%s</span>: %s", translation->or_word, hint);
                                    } else {
                                        snprintf(newtext, total_len + 49, "<span color='#FF0000'>%s</span>: %s", translation->or_word, translation->tr_word);

                                        if (wrong_answer_num == 5) {
                                            // Add inner vertical box for sentences
                                            GtkWidget* inner_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
                                            assert(inner_box);

                                            // Sentences
                                            GtkWidget* label_sentence1 = gtk_label_new("enter sentence");
                                            assert(label_sentence1);
                                            GtkWidget* entry_sentence1 = gtk_entry_new();
                                            assert(entry_sentence1);
                                            gtk_box_pack_start(GTK_BOX(inner_box), label_sentence1, FALSE, FALSE, 0);
                                            gtk_box_pack_start(GTK_BOX(inner_box), entry_sentence1, FALSE, FALSE, 0);
                                            g_signal_connect(GTK_ENTRY(entry_sentence1), "activate", G_CALLBACK(save_sentence), translation->or_word);

                                            gtk_container_add(GTK_CONTAINER(vbox), inner_box);
                                        }
                                    }

                                    safe_free((void**) &hint);
                                    ++wrong_answer_num;
                                } else {
                                    snprintf(newtext, strlen(current_label_text) + 49, "<span color='#FF0000'>%s</span>", translation->or_word);
                                }

                                GtkWidget* new_label = gtk_label_new(NULL);
                                assert(new_label);
                                gtk_label_set_markup(GTK_LABEL(new_label), newtext);

                                gtk_container_remove(GTK_CONTAINER(vbox_iter->data), GTK_WIDGET(eventbox_elements->data));
                                gtk_container_add(GTK_CONTAINER(vbox_iter->data), new_label);
                                gtk_widget_show_all(vbox_iter->data);
                                gtk_widget_show_all(vbox);

                                safe_free((void**) &newtext);
                                g_list_free(eventbox_view);
                                g_list_free(vbox_widgets);
                                return;
                            }
                        }
                        wrong_answer_num = 0;
                    }
                }
                g_list_free(eventbox_view);
            }
            g_list_free(vbox_widgets);
        }

        bool need_to_allocate = false;
        GtkWidget* main_window = get_main_window(need_to_allocate);

        clear_children(main_window);
        debug_test("redraw_arg->database_file: %s", arg->database_file);
        draw_main_screen(arg->database_file);
    }

    safe_free((void**) &translation);
    safe_free((void**) &arg);
}

static void draw_main_screen(const char* database_file)
{
    assert(database_file);
    translation_t* translation = NULL;
    if ((translation = pick_rand_translation(database_file)) == NULL) {
        return;
    }

    bool need_to_allocate = false;
    GtkWidget* main_window = get_main_window(need_to_allocate);

    debug_info("Retrieved translation: %s(%s) > %s(%s)\n",
            translation->or_word,
            translation->or_lang_code,
            translation->tr_word,
            translation->tr_lang_code);

    // Start drawing result
    GtkWidget *button = NULL, *grid = NULL, *vbox = NULL, *scroll = NULL;

    scroll = gtk_scrolled_window_new(NULL, NULL);
    assert(scroll);

    gtk_container_add(GTK_CONTAINER(main_window), scroll);
    gtk_widget_set_name(scroll, "main_scroll");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
            GTK_POLICY_AUTOMATIC,
            GTK_POLICY_AUTOMATIC);

    grid = gtk_grid_new();
    assert(grid);
    gtk_widget_set_name(grid, "main_grid");

    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);

    gtk_container_add(GTK_CONTAINER(scroll), grid);

    // Add inner vertical box with info per original word + translation + three sentences
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    assert(vbox);

    // Event box to capture clicks per word
    GtkWidget* eventbox_word = gtk_event_box_new();
    assert(eventbox_word);
    gtk_box_pack_start(GTK_BOX(vbox), eventbox_word, FALSE, FALSE, 0);

    GtkWidget* word_label = gtk_label_new(NULL);
    assert(word_label);
    gtk_label_set_markup(GTK_LABEL(word_label), translation->or_word);
    gtk_container_add(GTK_CONTAINER(eventbox_word), word_label);

    // Add context menus for words
    GtkWidget* parent_menu = gtk_menu_new();
    assert(parent_menu);

    GtkWidget* submenu_option_remove_word = gtk_menu_item_new_with_label("remove from DB");
    assert(submenu_option_remove_word);
    gtk_widget_show(submenu_option_remove_word);
    gtk_menu_shell_append(GTK_MENU_SHELL(parent_menu), submenu_option_remove_word);

    GtkWidget* submenu_option_about = gtk_menu_item_new_with_label("about");
    assert(submenu_option_about);
    gtk_widget_show(submenu_option_about);
    gtk_menu_shell_append(GTK_MENU_SHELL(parent_menu), submenu_option_about);

    g_signal_connect_swapped(G_OBJECT(eventbox_word), "button-press-event",
            G_CALLBACK(draw_popup_menu), parent_menu);
    gtk_grid_attach(GTK_GRID(grid), vbox, 0, 1, 1, 1);

    // Answer box
    GtkWidget* entry = gtk_entry_new();
    assert(entry);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 2, 1, 1);

    struct redraw_vbox_arg* redraw_arg = safe_alloc(sizeof(struct redraw_vbox_arg));
    redraw_arg->box = vbox;
    redraw_arg->translation = translation;
    redraw_arg->database_file = safe_alloc(strlen(database_file) + 1); // Will be freed at the bottom
    strncpy(redraw_arg->database_file, database_file, strlen(database_file));
    debug_test("redraw_arg->database_file: %s", redraw_arg->database_file);

    g_signal_connect(GTK_ENTRY(entry), "activate", G_CALLBACK(redraw_vbox), redraw_arg);

    button = gtk_button_new_with_label("next");
    assert(button);

    g_signal_connect(button, "clicked", G_CALLBACK(redraw_vbox), redraw_arg);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 1, 1);
    gtk_widget_set_name(button, "next");

    gtk_widget_show_all(main_window);
    safe_free((void**) &database_file);
}

static void fetch_entries(GtkEntry* entry, gpointer data)
{
    assert(data);
    struct GtkEntries* entries = data;

    bool need_to_allocate = false;
    spreadsheet_t* spreadsheet = get_spreadsheet(need_to_allocate);
    GtkWidget* main_window = get_main_window(need_to_allocate);

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
                    debug_info("Read Spreadsheet Key %s\n", text);
                    if (strlen(spreadsheet->key)) {
                        memset(spreadsheet->key, 0, MAX_ENTRY_LENGTH);
                    }
                    strncpy(spreadsheet->key, text, text_len);

                } else if (strcmp(type, "spreadsheet_gid_entry") == 0) {
                    debug_info("Read Spreadsheet GID %s\n", text);
                    if (strlen(spreadsheet->gid)) {
                        memset(spreadsheet->gid, 0, MAX_ENTRY_LENGTH);
                    }
                    strncpy(spreadsheet->gid, text, text_len);
                } else {
                    debug_error("text_type unknown %s, doing nothing\n", type);
                }
            }
        }
    }

    if (strlen(spreadsheet->key) && strlen(spreadsheet->gid)) {
        debug_fulldbg("Spreadsheet Key length %zu, Spreadsheet GID length %zu\n", strlen(spreadsheet->key), strlen(spreadsheet->gid));

        size_t mode = NEED_TO_CREATE;
        const char* database_file = NULL;
        if ((database_file = db_file(mode)) == NULL) {
            safe_free((void**) &entries);
            safe_free((void**) &spreadsheet);
            return;
        }

        if (!populate_database(database_file)) {
            return;
        }
        safe_free((void**) &entries);
        safe_free((void**) &spreadsheet);

        clear_children(main_window);
        draw_main_screen(database_file);
        gtk_widget_show_all(main_window);
    }
}

static void exit_gui(void)
{
    bool need_to_allocate = false;
    threadpool_t* pool = get_threadpool(need_to_allocate);
    debug_info("exiting, will stop threadpool at %p", (void*) pool);

    // Shutdown the thread pool
    size_t flags = threadpool_graceful;
    ssize_t res = threadpool_destroy(pool, flags);
    if (res != EXIT_SUCCESS) {
        debug_error("threadpool_destroy failed: %zi", res);
    }

    gtk_main_quit();
}

static void draw_csv_sync_screen(GtkWidget* widget, gpointer data)
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

static void draw_csv_sync_invite(void)
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

size_t draw_gui(void)
{
    size_t res = EXIT_SUCCESS;

    bool need_to_allocate = true;
    threadpool_t* pool = get_threadpool(need_to_allocate);
    debug_info("will use threadpool at %p", (void*) pool);

    // Main window
    GtkWidget* main_window = get_main_window(need_to_allocate);
    gtk_widget_set_name(main_window, "Translator - IELTS helper");

    gtk_window_set_default_size(GTK_WINDOW(main_window), 460, 230);
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
