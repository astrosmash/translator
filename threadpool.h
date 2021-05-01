#include <pthread.h>

typedef struct {
    void (*function)(void *);

    void *argument;
} threadpool_task_t;

struct threadpool_t {
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
};


// Definition
#include "threadpool.c"