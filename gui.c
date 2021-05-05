size_t draw_gui(threadpool_t* pool)
{
    assert(pool);
    size_t res = EXIT_SUCCESS;

    debug(DEBUG_INFO, "will use threadpool at %p", pool);


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

    gtk_widget_show_all(main_window);


    return res;
}

void exit_gui(gpointer data, GtkWidget* widget)
{
    assert(data);
    threadpool_t* pool = data;
    debug(DEBUG_INFO, "exiting, will stop threadpool at %p", pool);

    // Shutdown the thread pool
    size_t flags = threadpool_graceful;
    ssize_t res = threadpool_destroy(pool, flags);
    if (res != EXIT_SUCCESS) {
        debug(DEBUG_ERROR, "threadpool_destroy failed: %zi", res);
    }

    gtk_main_quit();
}