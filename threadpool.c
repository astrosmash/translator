// Internal auxiliary functions
// Function executed by each thread in the pool

static void* threadpool_thread(void *threadpool)
{
}

// Release the memory requested by the thread pool

bool threadpool_free(threadpool_t* pool)
{
}


// Create a threadpool, using Number of threads in the array and Number of tasks in the taskqueue array as arguments.
// Flags are currently unused.

threadpool_t* threadpool_create(size_t thread_count, size_t queue_size, size_t flags)
{
    assert(thread_count > MAX_THREADS && queue_size > MAX_QUEUE);

    threadpool_t* pool = NULL;
    assert((pool = safe_alloc(sizeof (threadpool_t))));

    // Initialize the pool
    pool->queue_size = queue_size;
    pool->threads = (pthread_t *) safe_alloc(sizeof (pthread_t) * thread_count);
    pool->threads_args = (pthread_attr_t *) safe_alloc(sizeof (pthread_attr_t) * thread_count);
    pool->queue = (threadpool_task_t *) safe_alloc(sizeof (threadpool_task_t) * thread_count);

    // Initialize mutex and conditional variables
    ssize_t res = 0;
    if ((res = pthread_mutex_init(&(pool->mutex), NULL))) {
        debug(DEBUG_ERROR, "pthread_mutex_init failed %zi", res);
        goto cleanup;
    }

    if ((res = pthread_cond_init(&(pool->condition), NULL))) {
        debug(DEBUG_ERROR, "pthread_cond_init failed %zi", res);
        goto cleanup;
    }

    // Start worker threads
    for (size_t i = 0; i <= thread_count; ++i) {
        // Initialize attr for specific thread
        if ((res = pthread_attr_init(&(pool->threads_args[i])))) {
            debug(DEBUG_ERROR, "pthread_attr_init failed %zi for %zu", res, i);
            goto cleanup;
        }

        if ((res = pthread_create(&(pool->threads[i]), &(pool->threads_args[i]), threadpool_thread, pool))) {
            debug(DEBUG_ERROR, "pthread_create failed %zi for %zu", res, i);
            if (!threadpool_destroy(pool, threadpool_immediate)) {
                debug(DEBUG_ERROR, "threadpool_destroy failed! %zu", i);
            }
            goto cleanup;
        }

        if ((res = pthread_attr_destroy(&(pool->threads_args[i])))) {
            debug(DEBUG_ERROR, "pthread_attr_destroy failed %zi for %zu", res, i);
            goto cleanup;
        }
        ++pool->count;
        ++pool->started;
    }

    return pool;

cleanup:
    safe_free((void**) &pool->queue);
    safe_free((void**) &pool->threads_args);
    safe_free((void**) &pool->threads);
    safe_free((void**) &pool);
    return NULL;
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
