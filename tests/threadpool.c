#define DEBUG_LEVEL DEBUG_TEST

#include "../main.h"

// threadpool_create()
threadpool_t* threadpool_create_test(void) {

    size_t thread_count = MAX_THREADS-1;
    size_t queue_size = MAX_QUEUE/2;
    size_t flags = 0;

    threadpool_t* testpool = threadpool_create(thread_count, queue_size, flags);
    assert(testpool);

    debug(DEBUG_TEST, "checking testpool->thread_count == thread_count: %zu == %zu", testpool->thread_count, thread_count);
    assert(testpool->thread_count == thread_count);
    debug(DEBUG_TEST, "checking testpool->queue_size == queue_size: %zu == %zu", testpool->queue_size, queue_size);
    assert(testpool->queue_size == queue_size);

//    queue_size = MAX_QUEUE + 1;
//    threadpool_t* testpool2 = threadpool_create(thread_count, queue_size, flags);
//    assert(!testpool2);

    return testpool;
}

// threadpool_add()
void* threadpool_task1(void* arg1)
{
    debug(DEBUG_TEST, "task1 %s", arg1);

    // this SHOULD be printed on console as we print task return value
    // We compare its first byte with char 't', this should be always true within tests
    return "threadpool_task1 ok\n";
}
void threadpool_add_test(threadpool_t* pool)
{
    size_t flags = 0;
    size_t res = EXIT_SUCCESS;

    char* threadpool_task1_arg1 = "success 1\n";
    char* threadpool_task1_arg2 = "success 2\n";
    debug(DEBUG_TEST, "arg1 %p, arg2 %p", threadpool_task1_arg1, threadpool_task1_arg2);

    assert(strcmp(threadpool_task1(threadpool_task1_arg1), "threadpool_task1 ok"));
    assert(strcmp(threadpool_task1(threadpool_task1_arg1), "threadpool_task1 ok\n") == 0);
    assert(strcmp(threadpool_task1(threadpool_task1_arg2), "threadpool_task1 ok\n") == 0);

    threadpool_task_t task1 = {&threadpool_task1, threadpool_task1_arg1};
    res = threadpool_add(pool, &task1, flags);
    assert(res == EXIT_SUCCESS);
    assert(pool->count == 1);

    assert(pool->queue[0].function == &threadpool_task1);
    assert(strcmp(pool->queue[0].argument, threadpool_task1_arg1) == 0);
    assert(strcmp(pool->queue[0].argument, "fake"));

    threadpool_task_t task2 = {&threadpool_task1, threadpool_task1_arg2};
    res = threadpool_add(pool, &task2, flags);
    assert(res == EXIT_SUCCESS);
    assert(pool->count == 2);

    assert(pool->queue[1].function == &threadpool_task1);
    assert(strcmp(pool->queue[1].argument, threadpool_task1_arg2) == 0);
    assert(strcmp(pool->queue[1].argument, pool->queue[0].argument));
}

// threadpool_destroy()
void threadpool_destroy_test_graceful(threadpool_t* pool)
{
    size_t flags = threadpool_graceful;
    size_t res = EXIT_SUCCESS;

    assert(!pool->shutdown);

    res = threadpool_destroy(pool, flags);
    assert(res == EXIT_SUCCESS);
//    assert(pool->shutdown == threadpool_shutdown_graceful);
    debug(DEBUG_TEST, "still have dangling pool ptr %p", pool);
}
void threadpool_destroy_test_immediate(threadpool_t* pool)
{
    size_t flags = threadpool_immediate;
    size_t res = EXIT_SUCCESS;

    assert(!pool->shutdown);

    res = threadpool_destroy(pool, flags);
    assert(res == EXIT_SUCCESS);
//    assert(pool->shutdown == threadpool_shutdown_immediate);
    debug(DEBUG_TEST, "still have dangling pool ptr %p", pool);
}

// threadpool_free()
void threadpool_free_test(threadpool_t* pool)
{
    size_t res = EXIT_SUCCESS;

    assert(pool);
    res = threadpool_free(pool);
    assert(res == EXIT_SUCCESS);
//    assert(!pool);
}

int main(int argc, char** argv)
{
    debug(DEBUG_INFO, "Starting... %c", '\0');

    threadpool_t* testpool_immediate = threadpool_create_test();
    threadpool_add_test(testpool_immediate);
    threadpool_destroy_test_immediate(testpool_immediate);

    threadpool_t* testpool_graceful = threadpool_create_test();
    threadpool_add_test(testpool_graceful);
    threadpool_destroy_test_graceful(testpool_graceful);

    threadpool_t* testpool_free = threadpool_create_test();
    threadpool_free_test(testpool_free);

    // Cleanup
    debug(DEBUG_TEST, "Exiting, track_block should not indicate any leftovers now... %c", '\0');
    if (track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_SUCCESS);
    }
    debug(DEBUG_ERROR, "track_block failed! %c", '\0');
    return (EXIT_FAILURE);
}
