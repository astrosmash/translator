#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <curl/curl.h>

// Inclusive logging output
// 1: (real) errors
#define DEBUG_ERROR 1
#define debug_error(fmt, ...)                                                                  \
    do {                                                                                      \
        if (DEBUG && DEBUG_LEVEL >= DEBUG_ERROR)                                                      \
            fprintf(stderr, "\n[ERROR] %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)
// 2: warnings about possibly not implemented data fields
#define DEBUG_WARN 2
#define debug_warn(fmt, ...)                                                                  \
    do {                                                                                      \
        if (DEBUG && DEBUG_LEVEL >= DEBUG_WARN)                                                      \
            fprintf(stderr, "\n[WARNING] %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)
// 3: verbose info about parsed fields / configurations that were set
#define DEBUG_INFO 3
#define debug_info(fmt, ...)                                                                  \
    do {                                                                                      \
        if (DEBUG && DEBUG_LEVEL >= DEBUG_INFO)                                                      \
            fprintf(stderr, "\n[INFO] %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)
// 4: iteration numbers and info about current location in nested loops
#define DEBUG_VERBOSE 4
#define debug_verbose(fmt, ...)                                                                  \
    do {                                                                                      \
        if (DEBUG && DEBUG_LEVEL >= DEBUG_VERBOSE)                                                      \
            fprintf(stderr, "\n[VERBOSE] %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)
// 5: full trace of received content
#define DEBUG_FULLDBG 5
#define debug_fulldbg(fmt, ...)                                                                  \
    do {                                                                                      \
        if (DEBUG && DEBUG_LEVEL >= DEBUG_FULLDBG)                                                      \
            fprintf(stderr, "\n[FULLDBG] %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)

// 6: full trace of test execution
#define DEBUG_TEST 6
#define debug_test(fmt, ...)                                                                  \
    do {                                                                                      \
        if (DEBUG && DEBUG_LEVEL >= DEBUG_TEST)                                                      \
            fprintf(stderr, "\n[TEST] %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)

#define DEBUG 1
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_TEST
#endif

typedef enum {
    false = 0,
    true
} bool;

#include "mempool.h"
#include "threadpool.h"
#include "gui.h"
