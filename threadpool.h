#include <pthread.h>

// (Mainly) from:
// https://programmer.group/c-simple-thread-pool-based-on-pthread-implementation.html


// Maximum number of members in the thread array
// Should not be more than number of CPU cores, in general
// PROC_COUNT is defined in the Makefile (getconf _NPROCESSORS_ONLN)
#define MAX_THREADS __NR_CPUS__

// Maximum number of members in the task queue array
#define MAX_QUEUE 256

typedef struct {
    // Worker function
    void (*function)(void *);

    // Arguments to the worker function
    void *argument;
} threadpool_task_t;

typedef struct {
    // Mutex
    pthread_mutex_t mutex;

    // Conditional variable to track
    pthread_cond_t condition;

    // Starting pointer of thread array
    pthread_t* threads;

    // Starting pointer of thread arguments array
    pthread_attr_t* threads_args;

    // Starting pointer of task queue array
    threadpool_task_t* queue;

    // Number of threads in the array
    size_t thread_count;

    // Number of tasks in the task queue array
    size_t queue_size;

    // Task queue head
    size_t head;

    // Task queue tail
    size_t tail;

    // Number of tasks to run currently
    size_t count;

    // If current state of thread pool is shutdown
    size_t shutdown;

    // Number of threads running
    size_t started;
} threadpool_t;


// Thread pool flags

typedef enum {
    threadpool_immediate = 1,
    threadpool_graceful
} threadpool_destroy_flags_t;

// Thread pool error codes

typedef enum {
    threadpool_invalid = -1,
    threadpool_lock_failed = -2,
    threadpool_queue_full = -3,
    threadpool_shutdown = -4,
    threadpool_thread_failed = -5
} threadpool_error_t;


// External functions
threadpool_t* threadpool_create(size_t, size_t, size_t);
bool threadpool_add(threadpool_t*, threadpool_task_t*, size_t);
bool threadpool_destroy(threadpool_t*, size_t);

// Definition
#include "threadpool.c"
