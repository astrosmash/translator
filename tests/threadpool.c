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
void threadpool_task1(void* arg1)
{
    debug(DEBUG_TEST, "task1 %s", arg1);
}
void threadpool_add_test(threadpool_t* pool)
{
    size_t flags = 0;
    size_t res = EXIT_SUCCESS;

    char* threadpool_task1_arg1 = "success 1\n";
    char* threadpool_task1_arg2 = "success 2\n";
    debug(DEBUG_TEST, "arg1 %p, arg2 %p", threadpool_task1_arg1, threadpool_task1_arg2);

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

int main(int argc, char** argv)
{
    debug(DEBUG_INFO, "Starting... %c", '\0');

    threadpool_t* testpool = threadpool_create_test();
    threadpool_add_test(testpool);

    return (EXIT_SUCCESS);
}
