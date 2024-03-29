// Internal auxiliary functions
// Function executed by each thread in the pool
static void* threadpool_thread(void *threadpool)
{
    assert(threadpool);
    threadpool_t* pool = (threadpool_t *) threadpool;
    threadpool_task_t task = {.argument = NULL, .function = NULL};
    debug_test("PRE started threads: %zu", pool->started);

    do {
        // Lock the mutex to get the conditional variable
        if (pthread_mutex_lock(&(pool->mutex))) {
            debug_error("pthread_mutex_lock failed. Tasks count: %zu", pool->count);
            return NULL;
        }

        // Wait on conditional variable
        while (pool->count == 0 && !pool->shutdown) {
            // Task queue is empty.
            // Thread pool is locked when not used
            if (pthread_cond_wait(&(pool->condition), &(pool->mutex))) {
                debug_info("pthread_cond_wait failed. Tasks count: %zu", pool->count);
                return NULL;
            }
            debug_test("pthread_cond_wait ok. Tasks count: %zu", pool->count);
        }

        // Got outside of the while loop - stop processing
        if ((pool->shutdown == threadpool_shutdown_immediate) ||
                (pool->shutdown == threadpool_graceful && pool->count == 0)) {
            debug_info("Closing processing. Tasks count: %zu"
                    "(threadpool_shutdown_immediate if > 0)", pool->count);
            break;
        }

        debug_test("PRE pool->head: %zu pool->count: %zu", pool->head, pool->count);
        // Get the first task from the task queue.
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;

        // Update head and count
        ++pool->head;
        // Set head to zero if we are in the bottom of the pool
        pool->head = (pool->queue_size == pool->head) ? 0 : pool->head;
        --pool->count;
        debug_test("POST pool->head: %zu tasks->count: %zu", pool->head, pool->count);

        // Finally, unlock the mutex
        if (pthread_mutex_unlock(&(pool->mutex))) {
            debug_error("pthread_mutex_unlock failed. Tasks count: %zu", pool->count);
            return NULL;
        }
        // Start the function
        (void) (*(task.function)(task.argument));

    } while (true);

    // Update the number of running threads
    if (pool->started) {
        --pool->started;
    }
    debug_test("POST started threads: %zu", pool->started);

    // Unlock the mutex - just in case it's locked
    if (pthread_mutex_unlock(&(pool->mutex))) {
        debug_error("pthread_mutex_unlock failed. Tasks count: %zu", pool->count);
        return NULL;
    }

    pthread_exit(NULL);
    return (NULL);
}

// Release the memory requested by the thread pool
ssize_t threadpool_free(threadpool_t* pool)
{
    ssize_t result = EXIT_SUCCESS;
    assert(pool && pool->started == 0);

    if (pool->threads) {
        if (pthread_mutex_lock(&(pool->mutex))) {
            debug_error("pthread_mutex_lock failed. Tasks count: %zu", pool->count);
            result = threadpool_lock_failure;
            return result;
        }
        debug_test("pthread_mutex_lock %zi\n", result);

        if (pthread_mutex_unlock(&(pool->mutex))) {
            debug_error("pthread_mutex_unlock failed. Tasks count: %zu", pool->count);
            result = threadpool_lock_failure;
            return result;
        }
        debug_test("pthread_mutex_unlock %zi\n", result);

        if (pthread_mutex_destroy(&(pool->mutex))) {
            debug_error("pthread_mutex_destroy failed. Tasks count: %zu", pool->count);
            result = threadpool_lock_failure;
            return result;
        }
        debug_test("pthread_mutex_destroy %zi\n", result);

        if (pthread_cond_destroy(&(pool->condition))) {
            debug_error("pthread_cond_destroy failed. Tasks count: %zu", pool->count);
            result = threadpool_lock_failure;
            return result;
        }
        debug_test("pthread_cond_destroy %zi\n", result);

        safe_free((void**) &pool->queue);
        safe_free((void**) &pool->threads_attrs);
        safe_free((void**) &pool->threads);
    }
    safe_free((void**) &pool);

    debug_test("threadpool_free done, exiting. %zi\n", result);
    return result;
}


// Create a threadpool, using Number of threads in the array and Number of tasks in the taskqueue array as arguments.
// Flags are currently unused.
threadpool_t* threadpool_create(size_t thread_count, size_t queue_size, size_t flags)
{
    assert(thread_count <= MAX_THREADS && queue_size <= MAX_QUEUE);
    threadpool_t* pool = NULL;
    assert((pool = safe_alloc(sizeof (threadpool_t))));

    // Initialize the pool
    pool->queue_size = queue_size;

    pool->threads = (pthread_t *) safe_alloc(sizeof (pthread_t) * thread_count);
    debug_test("Allocated pool->threads on %p\n", (void *) pool->threads);

    pool->threads_attrs = (pthread_attr_t *) safe_alloc(sizeof (pthread_attr_t) * thread_count);
    debug_test("Allocated pool->threads_attrs on %p\n", (void *) pool->threads_attrs);

    pool->queue = (threadpool_task_t *) safe_alloc(sizeof (threadpool_task_t) * thread_count);
    debug_test("Allocated pool->queue on %p\n", (void *) pool->queue);

    // Initialize mutex and conditional variables
    ssize_t res = 0;
    if ((res = pthread_mutex_init(&(pool->mutex), NULL))) {
        debug_error("pthread_mutex_init failed %zi", res);
        goto cleanup;
    }
    debug_test("pthread_mutex_init %zi\n", res);

    if ((res = pthread_cond_init(&(pool->condition), NULL))) {
        debug_error("pthread_cond_init failed %zi", res);
        goto cleanup;
    }
    debug_test("pthread_cond_init %zi\n", res);

    // Block SIGQUIT and SIGUSR1
    sigset_t signalset;
    sigemptyset(&signalset);
    sigaddset(&signalset, SIGQUIT);
    sigaddset(&signalset, SIGUSR1);

    // Set attributes of the threads (pool-wide)
    if ((res = pthread_sigmask(SIG_BLOCK, &signalset, NULL))) {
        debug_error("pthread_sigmask failed! %zi", res);
        goto cleanup;
    }
    debug_test("pthread_sigmask %zi\n", res);

    // Start worker threads
    for (size_t i = 0; i < thread_count; ++i) {
        // Initialize attr for specific thread
        if ((res = pthread_attr_init(&(pool->threads_attrs[i])))) {
            debug_error("pthread_attr_init failed %zi for %zu", res, i);
            goto cleanup;
        }
        debug_test("#%zu pthread_attr_init %zi\n", i, res);

        if ((res = pthread_create(&(pool->threads[i]), &(pool->threads_attrs[i]), threadpool_thread, pool))) {
            debug_error("pthread_create failed %zi for %zu", res, i);
            if (!threadpool_destroy(pool, threadpool_immediate)) {
                debug_error("threadpool_destroy failed! %zu", i);
            }
            goto cleanup;
        }
        debug_test("#%zu pthread_create %zi\n", i, res);

        if ((res = pthread_attr_destroy(&(pool->threads_attrs[i])))) {
            debug_error("pthread_attr_destroy failed %zi for %zu", res, i);
            goto cleanup;
        }
        debug_test("#%zu pthread_attr_destroy %zi\n", i, res);
        ++pool->thread_count;
        ++pool->started;
    }

    debug_test("initialized pool with count %zu started %zu res %zi\n", pool->count, pool->started, res);
    return pool;

cleanup:
    threadpool_free(pool);
    return NULL;
}

// Add task to the pool, using worker function and arguments for it.
// Flags are currently unused.
ssize_t threadpool_add(threadpool_t* pool, threadpool_task_t* task, size_t flags)
{
    assert(pool && task);
    ssize_t result = EXIT_SUCCESS;

    if (pthread_mutex_lock(&(pool->mutex))) {
        debug_error("pthread_mutex_lock failed. Tasks count: %zu", pool->count);
        result = threadpool_lock_failure;
        return result;
    }
    debug_test("pthread_mutex_lock %zi\n", result);

    // Next location to store the task
    size_t next = pool->tail + 1;
    next = (next == pool->queue_size) ? 0 : next;
    debug_test("pool->tail: %zu next %zu\n", pool->tail, next);

    // do  {} while (0) -- At most once.
    do {
        // Task queue is full.
        if (pool->queue_size == pool->count) {
            debug_error("Pool queue size is full. Tasks count: %zu", pool->count);
            result = threadpool_queue_full;
            break;
        }
        debug_test("pool->queue_size: %zu pool->count: %zu\n", pool->queue_size, pool->count);

        // Pool shutdown.
        if (pool->shutdown) {
            debug_info("Pool is shutting down. Tasks count: %zu", pool->count);
            result = threadpool_shutdown;
            break;
        }
        debug_test("pool->shutdown: %u\n", pool->shutdown);

        // Add task and its arguments to the tail of the queue.
        pool->queue[pool->tail].function = task->function;
        pool->queue[pool->tail].argument = task->argument;

        debug_test("#%zu func %p arg %p retval block %p\n", pool->tail,
                (void *) pool->queue[pool->tail].function,
                pool->queue[pool->tail].argument->arg,
                pool->queue[pool->tail].argument->block_to_store_retval);

        // Update tail and count
        pool->tail = next;
        ++pool->count;
        debug_test("pool->tail: %zu pool->count: %zu\n", pool->tail, pool->count);

        // Signal to indicate that the task has been added
        // The pthread_cond_broadcast() function shall unblock all threads currently blocked on the specified condition variable cond.
        // The pthread_cond_signal() function shall unblock at least one of the threads that are blocked on the specified condition variable cond (if any threads are blocked on cond).
        if (pthread_cond_signal(&(pool->condition))) {
            debug_info("pthread_cond_signal failed. Tasks count: %zu", pool->count);
            result = threadpool_lock_failure;
            break;
        }
        debug_test("pthread_cond_signal %zi\n", result);
    } while (0);
    // do  {} while (0) -- At most once.

    // Finally, unlock the mutex
    if (pthread_mutex_unlock(&(pool->mutex))) {
        debug_error("pthread_mutex_unlock failed. Tasks count: %zu", pool->count);
        result = threadpool_lock_failure;
    }
    debug_test("pthread_mutex_unlock %zi\n", result);

    return result;
}

// Destroy existing pool.
// Flags specify whether termination should be graceful or immediate (regardless of whether it's empty or not).
// Graceful termination waits for threads to join.
ssize_t threadpool_destroy(threadpool_t* pool, size_t flags)
{
    assert(pool);
    ssize_t result = EXIT_SUCCESS;

    if (pthread_mutex_lock(&(pool->mutex))) {
        debug_error("pthread_mutex_lock failed. Tasks count: %zu", pool->count);
        result = threadpool_lock_failure;
        return result;
    }
    debug_test("pthread_mutex_lock %zi\n", result);

    // do  {} while (0) -- At most once.
    do {
        // Check if the pool is already being shutdown.
        if (pool->shutdown) {
            debug_error("Pool is already being shutdown. Tasks count: %zu", pool->count);
            result = threadpool_shutdown;
            break;
        }
        pool->shutdown = (flags & threadpool_graceful) ? threadpool_shutdown_graceful : threadpool_shutdown_immediate;
        debug_test("pool->shutdown: %u\n", pool->shutdown);

        // Signal to indicate the shutdown
        if (pthread_cond_broadcast(&(pool->condition))) {
            debug_info("pthread_cond_broadcast failed. Tasks count: %zu", pool->count);
            result = threadpool_lock_failure;
            break;
        }
        debug_test("pthread_cond_broadcast %zi\n", result);

        if (pthread_mutex_unlock(&(pool->mutex))) {
            debug_error("pthread_mutex_unlock failed. Tasks count: %zu", pool->count);
            result = threadpool_lock_failure;
            break;
        }
        debug_test("pthread_mutex_unlock %zi\n", result);

        // Wait for the threads to terminate
        for (size_t i = 0; i < pool->thread_count; ++i) {
            if (pthread_join(pool->threads[i], NULL)) {
                debug_error("#%zu pthread_join failed. Tasks count: %zu", i, pool->count);
                result = threadpool_thread_failure;
                break;
            }
            debug_test("#%zu pthread_join/thread_result %zi\n", i, result);
        }
    } while (0);
    // do  {} while (0) -- At most once.

    if (result == EXIT_SUCCESS) {
        threadpool_free(pool);
    }

    return result;
}
