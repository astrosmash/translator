#define DEBUG_LEVEL DEBUG_TEST

#include "../main.h"

// get_spreadsheet()
void get_spreadsheet_test(void)
{
    bool need_to_allocate = true;
    spreadsheet_t* sheet = get_spreadsheet(need_to_allocate);

    need_to_allocate = false;
    spreadsheet_t* sheet2 = get_spreadsheet(need_to_allocate);

    assert(sheet == sheet2);

    safe_free((void**) &sheet);
    assert(!sheet);
}

// my_curl_init() + submit_curl_task()
void curl_test(void)
{
    struct curl_string s = {.len = 0};
    s.ptr = malloc(s.len + 1); // Don't track and free for now - managed by curl
    s.ptr[0] = '\0';

    const char* testcookie = "abcd=123";
    CURL* mycurl = my_curl_init(&s, testcookie);
    assert(mycurl);

    debug_test("Should see > my_curl_init(): Will use abcd=123 < above %c", '\n');
    curl_easy_cleanup(mycurl);

    // submit_curl_task()
    // performs curl_easy_cleanup() internally
    bool res = submit_curl_task("https://google.com/", testcookie, &s, NULL);
    assert(res);
}

// populate_translation()
void populate_translation_test(void)
{
    translation_t* translation = safe_alloc(sizeof (translation_t));

    const char* what1 = "английский,русский,fusil,фузея";
    char* buf = safe_alloc(strlen(what1) + 2); // strtok_r
    strncpy(buf, what1, strlen(what1));
    assert(populate_translation(buf, translation));
    safe_free((void**) &buf);

    assert(strcmp(translation->or_word, "fusil") == 0);
    assert(strcmp(translation->or_lang_code, "en") == 0);
    assert(strcmp(translation->tr_word, "фузея") == 0);
    assert(strcmp(translation->tr_lang_code, "ru") == 0);

    const char* what2 = "английский,русский,cradleboard,колыбель";
    buf = safe_alloc(strlen(what2) + 2);
    strncpy(buf, what2, strlen(what2));
    assert(populate_translation(buf, translation));
    safe_free((void**) &buf);

    assert(strcmp(translation->or_word, "cradleboard") == 0);
    assert(strcmp(translation->or_lang_code, "en") == 0);
    assert(strcmp(translation->tr_word, "колыбель") == 0);
    assert(strcmp(translation->tr_lang_code, "ru") == 0);

    const char* what3 = "русский,английский,увеличить вашу прибыль,ramp up your profits";
    buf = safe_alloc(strlen(what3) + 2);
    strncpy(buf, what3, strlen(what3));
    assert(populate_translation(buf, translation));
    safe_free((void**) &buf);

    assert(strcmp(translation->or_word, "увеличить вашу прибыль") == 0);
    assert(strcmp(translation->or_lang_code, "ru") == 0);
    assert(strcmp(translation->tr_word, "ramp up your profits") == 0);
    assert(strcmp(translation->tr_lang_code, "en") == 0);

    const char* what4 = "русский,английский,ВЛОв8932 кшвыу флодфывоыЩЩ,FJK FSDLK DKJDKHsdjk";
    buf = safe_alloc(strlen(what4) + 2);
    strncpy(buf, what4, strlen(what4));
    assert(populate_translation(buf, translation));
    safe_free((void**) &buf);

    assert(strcmp(translation->or_word, "ВЛОв8932 кшвыу флодфывоыЩЩ") == 0);
    assert(strcmp(translation->or_lang_code, "ru") == 0);
    assert(strcmp(translation->tr_word, "FJK FSDLK DKJDKHsdjk") == 0);
    assert(strcmp(translation->tr_lang_code, "en") == 0);

    safe_free((void**) &translation);
}

// parse_csv()
void parse_csv_test(void)
{
    char* csv = "английский,русский,fusil,фузея\n"
        "английский,русский,cradleboard,колыбель\n"
        "русский,английский,увеличить вашу прибыль,ramp up your profits\n"
        "русский,английский,ВЛОв8932 кшвыу флодфывоыЩЩ,FJK FSDLK DKJDKHsdjk";
    char* buf = safe_alloc(strlen(csv) + 2); // strtok_r
    strncpy(buf, csv, strlen(csv));

    translation_response_t* translations = parse_csv(buf);
    assert(translations);

    assert(strcmp(translations->result[3]->or_word, "ВЛОв8932 кшвыу флодфывоыЩЩ") == 0);
    assert(strcmp(translations->result[3]->or_lang_code, "ru") == 0);
    assert(strcmp(translations->result[3]->tr_word, "FJK FSDLK DKJDKHsdjk") == 0);
    assert(strcmp(translations->result[3]->tr_lang_code, "en") == 0);

    assert(strcmp(translations->result[2]->or_word, "увеличить вашу прибыль") == 0);
    assert(strcmp(translations->result[2]->or_lang_code, "ru") == 0);
    assert(strcmp(translations->result[2]->tr_word, "ramp up your profits") == 0);
    assert(strcmp(translations->result[2]->tr_lang_code, "en") == 0);

    assert(strcmp(translations->result[1]->or_word, "cradleboard") == 0);
    assert(strcmp(translations->result[1]->or_lang_code, "en") == 0);
    assert(strcmp(translations->result[1]->tr_word, "колыбель") == 0);
    assert(strcmp(translations->result[1]->tr_lang_code, "ru") == 0);

    assert(strcmp(translations->result[0]->or_word, "fusil") == 0);
    assert(strcmp(translations->result[0]->or_lang_code, "en") == 0);
    assert(strcmp(translations->result[0]->tr_word, "фузея") == 0);
    assert(strcmp(translations->result[0]->tr_lang_code, "ru") == 0);

    assert(strcmp(translations->result[3]->or_word, "fake"));
    assert(strcmp(translations->result[2]->or_word, "fake"));
    assert(strcmp(translations->result[1]->tr_word, "fake"));
    assert(strcmp(translations->result[0]->tr_word, "fake"));

    // Record some data for latter pick_rand_translation_test()
    const char* database_file = ".db";
    DB *db_p = NULL;
    ssize_t db_ret = 0;
    if ((db_ret = db_create(&db_p, NULL, 0))) {
        debug_error("Cannot db_create (%s)\n", db_strerror(db_ret));
        return;
    }
    assert(db_p);

    size_t db_flags = DB_CREATE;
    size_t db_mode = DB_HASH;
    if ((db_ret = db_p->open(db_p, NULL, database_file, NULL, db_mode, db_flags, 0600))) {
        debug_error("Cannot db_p->open (%s)\n", db_strerror(db_ret));
        return;
    }

    for (size_t i = 0; i <= translations->num_of_translations; ++i) {
        DBT key;
        memset(&key, 0, sizeof (key));
        DBT value;
        memset(&value, 0, sizeof (value));

        key.data = &i;
        key.size = sizeof (i);
        value.data = translations->result[i];
        value.size = sizeof (*translations->result[i]);

        if ((db_ret = db_p->put(db_p, NULL, &key, &value, 0))) {
            debug_error("Cannot db_p->put (%s)\n", db_strerror(db_ret));
            return;
        }

        if ((db_ret = db_p->get(db_p, NULL, &key, &value, 0))) {
            debug_error("Cannot db_p->get (%s)\n", db_strerror(db_ret));
            return;
        }

        safe_free((void**) &translations->result[i]);
    }

    if ((db_ret = db_p->close(db_p, 0))) {
        debug_error("Cannot db_p->close (%s)\n", db_strerror(db_ret));
        return;
    }

    safe_free((void**) &translations);
    safe_free((void**) &buf);
}

// db_file() + get_homedir()
void db_file_test(void)
{
    const char* homedir = get_homedir();

    size_t mode = NEED_TO_CHECK;
    const char* database_file = NULL;

    if ((database_file = db_file(mode)) == NULL) {
        mode = NEED_TO_CREATE; // will automatically unlink
        assert((database_file = db_file(mode)) != NULL);
    }

    debug_test("homedir: %s, database_file: %s", homedir, database_file);
    assert(strstr(database_file, homedir));
    assert(strstr(database_file, "/.tiny-ielts/.db"));
}

// pick_rand_translation()
void pick_rand_translation_test(void)
{
    // Will not test random, but will check all database entries
    const char* database_file = ".db";

    DB *db_p = NULL;
    ssize_t db_ret = 0;
    if ((db_ret = db_create(&db_p, NULL, 0))) {
        debug_error("Cannot db_create (%s)\n", db_strerror(db_ret));
        return;
    }
    assert(db_p);

    size_t db_flags = DB_RDONLY;
    size_t db_mode = DB_HASH;
    if ((db_ret = db_p->open(db_p, NULL, database_file, NULL, db_mode, db_flags, 0400))) {
        debug_error("Cannot db_p->open (%s)\n", db_strerror(db_ret));
        return;
    }

    DB_HASH_STAT* stat;
    if ((db_ret = db_p->stat(db_p, NULL, &stat, 0))) {
        debug_error("Cannot db_p->stat (%s)\n", db_strerror(db_ret));
        return;
    }
    debug_verbose("Got %u keys\n", stat->hash_nkeys);
    assert(stat->hash_nkeys == 4);

    DBT key;
    memset(&key, 0, sizeof (key));
    DBT value;
    memset(&value, 0, sizeof (value));


    size_t keypos = 0;
    key.data = &keypos;
    key.size = sizeof (keypos);

    if ((db_ret = db_p->get(db_p, NULL, &key, &value, 0))) {
        debug_error("Cannot db_p->get (%s)\n", db_strerror(db_ret));
        return;
    }

    translation_t* translation = (translation_t *) value.data;
    assert(strcmp(translation->or_word, "fusil") == 0);
    assert(strcmp(translation->or_lang_code, "en") == 0);
    assert(strcmp(translation->tr_word, "фузея") == 0);
    assert(strcmp(translation->tr_lang_code, "ru") == 0);

    keypos = 1;
    key.data = &keypos;
    key.size = sizeof (keypos);

    if ((db_ret = db_p->get(db_p, NULL, &key, &value, 0))) {
        debug_error("Cannot db_p->get (%s)\n", db_strerror(db_ret));
        return;
    }

    translation = (translation_t *) value.data;
    assert(strcmp(translation->or_word, "cradleboard") == 0);
    assert(strcmp(translation->or_lang_code, "en") == 0);
    assert(strcmp(translation->tr_word, "колыбель") == 0);
    assert(strcmp(translation->tr_lang_code, "ru") == 0);

    keypos = 2;
    key.data = &keypos;
    key.size = sizeof (keypos);

    if ((db_ret = db_p->get(db_p, NULL, &key, &value, 0))) {
        debug_error("Cannot db_p->get (%s)\n", db_strerror(db_ret));
        return;
    }

    translation = (translation_t *) value.data;
    assert(strcmp(translation->or_word, "увеличить вашу прибыль") == 0);
    assert(strcmp(translation->or_lang_code, "ru") == 0);
    assert(strcmp(translation->tr_word, "ramp up your profits") == 0);
    assert(strcmp(translation->tr_lang_code, "en") == 0);

    keypos = 3;
    key.data = &keypos;
    key.size = sizeof (keypos);

    if ((db_ret = db_p->get(db_p, NULL, &key, &value, 0))) {
        debug_error("Cannot db_p->get (%s)\n", db_strerror(db_ret));
        return;
    }

    translation = (translation_t *) value.data;
    assert(strcmp(translation->or_word, "ВЛОв8932 кшвыу флодфывоыЩЩ") == 0);
    assert(strcmp(translation->or_lang_code, "ru") == 0);
    assert(strcmp(translation->tr_word, "FJK FSDLK DKJDKHsdjk") == 0);
    assert(strcmp(translation->tr_lang_code, "en") == 0);

    assert(strcmp(translation->or_word, "fake"));
    assert(strcmp(translation->tr_word, "fake"));


    if ((db_ret = db_p->close(db_p, 0))) {
        debug_error("Cannot db_p->close (%s)\n", db_strerror(db_ret));
        return;
    }

    assert(unlink(database_file) == 0);
}

int main(int argc, char** argv)
{
    debug_info("Util test starting... %c", '\n');

    get_spreadsheet_test();
    curl_test();
    populate_translation_test();
    parse_csv_test();
    db_file_test();
    pick_rand_translation_test();

    // Cleanup
    debug_test("Exiting, track_block should not indicate any leftovers now... %c", '\0');
    if (track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_SUCCESS);
    }
    debug_error("track_block failed! %c", '\0');
    return (EXIT_FAILURE);
}
