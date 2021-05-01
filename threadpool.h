#include <pthread.h>

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

    // Starting pointer of taskqueue array
    threadpool_task_t queue;

    // Number of threads in the array
    size_t thread_count;

    // Number of tasks in the taskqueue array
    size_t queue_size;

    // Taskqueue head
    size_t head;

    // Taskqueue tail
    size_t tail;

    // Number of tasks to run currently
    size_t count;

    // If current state of threadpool is shutdown
    size_t shutdown;

    // Number of threads running
    size_t started;
} threadpool_t;

// Threadpool flags

typedef enum {
    threadpool_graceful = 1
} threadpool_destroy_flags_t;

// Threadpool error codes

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
