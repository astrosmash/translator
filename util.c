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


// Helper function for curl to write output.
static size_t curl_write_func(void* ptr, size_t size, size_t nmemb, struct curl_string* s)
{
    assert(ptr);
    assert(s);

    size_t total_size = size * nmemb;
    size_t new_len = s->len + total_size;

    s->ptr = realloc(s->ptr, new_len + 1);
    assert(s->ptr);

    memcpy(s->ptr + s->len, ptr, total_size);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return total_size;
}

// Helper function to init curl session.
// Curl object can be used later with custom per-function options.
static CURL* my_curl_init(struct curl_string* s, const char* cookie)
{
    CURL* curl = curl_easy_init();
    assert(curl);

    struct curl_slist* headers = NULL;
    char* user_agent = safe_alloc(MAX_ENTRY_LENGTH);

    if (!snprintf(user_agent, MAX_ENTRY_LENGTH - 2, "User-Agent: tiny-ielts/%f", 0.1)) {
        debug_error("Unable to populate User-Agent %s\n", user_agent);
        safe_free((void**) &user_agent);
        return NULL;
    }

    debug_info("Will use %s\n", user_agent);
    headers = curl_slist_append(headers, user_agent);
    assert(headers);

    if (cookie) {
        headers = curl_slist_append(headers, cookie);
        debug_info("Will use %s\n", cookie);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);

    safe_free((void**) &user_agent);
    return curl;
}

static bool submit_curl_task(const char* url, const char* cookie, struct curl_string* s, void* postfields)
{
    assert(url);
    assert(s);

    CURL* curl = my_curl_init(s, cookie);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    if (postfields) { // POST mode
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        debug_error("Got curl_easy_perform err %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return false;
    }

    size_t curl_esponse_code = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curl_esponse_code);
    if (res != CURLE_OK) {
        debug_error("Got curl_easy_getinfo err %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return false;
    }

    if (curl_esponse_code == 200) {
        debug_info("%s CURL result %s\n", url, s->ptr);
    } else {
        debug_error("CURL got HTTP error code %zu result %s\n", curl_esponse_code, s->ptr);
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_cleanup(curl);
    return true;
}

static const char* get_homedir(void)
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

bool populate_database(const char* database_file)
{
    assert(database_file);

    bool need_to_allocate = false;
    spreadsheet_t* spreadsheet = get_spreadsheet(need_to_allocate);
    threadpool_t* pool = get_threadpool(need_to_allocate);
    debug_fulldbg("will use spreadsheet at %p threadpool at %p", (void*) spreadsheet, (void*) pool);

    struct curl_string s = {.len = 0};
    s.ptr = malloc(s.len + 1); // Don't track and free for now - managed by curl
    s.ptr[0] = '\0';

    char* url = safe_alloc(MAX_ENTRY_LENGTH);
    if (!snprintf(url, MAX_ENTRY_LENGTH - 2, "https://docs.google.com/spreadsheets/d/%s/export?gid=%s&format=csv", spreadsheet->key, spreadsheet->gid)) {
        debug_error("Cannot assemble URL https://docs.google.com/spreadsheets/d/%s/export?gid=%s&format=csv", spreadsheet->key, spreadsheet->gid);
        safe_free((void**) &url);
        return false;
    }
    debug_info("Populated URL %s", url);

    bool res = submit_curl_task(url, NULL, &s, NULL);
    debug_info("submit_curl_task res: %u ", res);

    safe_free((void**) &url);
    return res;
}
