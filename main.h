#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Inclusive logging output
// 1: (real) errors
#define DEBUG_ERROR 1
// 2: warnings about possibly not implemented data fields
#define DEBUG_WARN 2
// 3: verbose info about parsed fields / configurations that were set
#define DEBUG_INFO 3
// 4: iteration numbers and info about current location in nested loops
#define DEBUG_VERBOSE 4
// 5: full trace of received content
#define DEBUG_FULLDBG 5
// 6: full trace of test execution
#define DEBUG_TEST 6
#define debug(lvl, fmt, ...)                                                                  \
    do {                                                                                      \
        if (DEBUG && DEBUG_LEVEL >= lvl)                                                      \
            fprintf(stderr, "\n[level %i] %s:%d:%s(): " fmt, lvl, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)

// Include debug output
#define DEBUG 1
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_FULLDBG
#endif

typedef enum {
    false = 0,
    true
} bool;

#include "mempool.h"
#include "threadpool.h"
