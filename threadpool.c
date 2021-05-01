// Create a threadpool, using Number of threads in the array and Number of tasks in the taskqueue array as arguments.
// Flags are currently unused.

threadpool_t* threadpool_create(size_t thread_count, size_t queue_size, size_t flags)
{
}

// Add task to the pool, using worker function and arguments for it.
// Flags are currently unused.

bool threadpool_add(threadpool_t* pool, threadpool_task_t* task, size_t flags)
{
}

// Destroy existing pool.
// Flags specify whether termination should be graceful or immediate (regardless of whether it's empty or not).
// Graceful termination waits for threads to join.

bool threadpool_destroy(threadpool_t* pool, size_t flags)
{
}

// Internal auxiliary functions
// Function executed by each thread in the pool

static void* threadpool_thread(void *threadpool)
{
}

// Release the memory requested by the thread pool

bool threadpool_free(threadpool_t* pool)
{
}