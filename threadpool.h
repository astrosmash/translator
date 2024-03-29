#include <pthread.h>
#include <signal.h>

// (Mainly) from:
// https://github.com/mbrossard/threadpool


// Maximum number of members in the thread array
// Should not be more than number of CPU cores, in general
// PROC_COUNT is defined in the Makefile (getconf _NPROCESSORS_ONLN)
#define MAX_THREADS __NR_CPUS__

// Maximum number of members in the task queue array
#define MAX_QUEUE 256

// Memory to allocate for thread retval
#define THREAD_RETVAL 2048

// Thread pool flags
typedef enum {
    threadpool_immediate = 1,
    threadpool_graceful
} threadpool_destroy_flags_t;

// Threadpool shutdown modes
typedef enum {
    threadpool_shutdown_immediate = 1,
    threadpool_shutdown_graceful
} threadpool_shutdown_t;

// Thread pool error codes
typedef enum {
    threadpool_queue_full = -1,
    threadpool_shutdown = -2,
    threadpool_thread_failure = -3,
    threadpool_lock_failure = -4
} threadpool_error_t;

typedef struct {
    // Actual argument
    void* arg;

    // Location to store retval per worker function
    void* block_to_store_retval;
} composite_arg_t;

typedef struct {
    // Worker function
    void* (*function)(composite_arg_t *);

    // Arguments to the worker function
    composite_arg_t *argument;
} threadpool_task_t;

typedef struct {
    // Mutex
    pthread_mutex_t mutex;

    // Conditional variable to track
    pthread_cond_t condition;

    // Starting pointer of thread array
    pthread_t* threads;

    // Starting pointer of thread arguments array
    pthread_attr_t* threads_attrs;

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
    threadpool_shutdown_t shutdown;

    // Number of threads running
    size_t started;
} threadpool_t;


// External functions
threadpool_t* threadpool_create(size_t, size_t, size_t);
ssize_t threadpool_add(threadpool_t*, threadpool_task_t*, size_t);
ssize_t threadpool_destroy(threadpool_t*, size_t);
ssize_t threadpool_free(threadpool_t*);

// Definition
#include "threadpool.c"
