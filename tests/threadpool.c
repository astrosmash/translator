#define DEBUG_LEVEL DEBUG_TEST

#include "../main.h"

// threadpool_create()
threadpool_t* threadpool_create_test(void)
{
    size_t thread_count = MAX_THREADS - 1;
    size_t queue_size = MAX_QUEUE / 2;
    size_t flags = 0;

    threadpool_t* testpool = threadpool_create(thread_count, queue_size, flags);
    assert(testpool);

    debug_test("checking testpool->thread_count == thread_count: %zu == %zu", testpool->thread_count, thread_count);
    assert(testpool->thread_count == thread_count);
    debug_test("checking testpool->queue_size == queue_size: %zu == %zu", testpool->queue_size, queue_size);
    assert(testpool->queue_size == queue_size);

    return testpool;
}

// threadpool_add()
#include <sys/types.h>
#include <sys/wait.h>

void* threadpool_task_icmp(composite_arg_t* arg1)
{
    void* arg = arg1->arg;
    void* retval = arg1->block_to_store_retval;

    debug_test("arg %s, block to store retval %p", arg, retval);

    if (retval) {
        strncpy(retval, arg, strlen(arg));
    }

    int childstatus = EXIT_SUCCESS;
    pid_t pid = fork();
    char* bin_ping = "/bin/ping";

    if (pid == 0) { // Forked child
        char *newargv[] = {bin_ping, "-c 10", "-s 100", arg, NULL};
        char *newenviron[] = {NULL};
        execve(bin_ping, newargv, newenviron);
    }

    if (pid > 0) { // Parent
        pid = wait(&childstatus);

        debug_test("child pid %d ", pid);
        if (WIFEXITED(childstatus)) {
            debug_test("exited with code %d", WEXITSTATUS(childstatus));
        }
        if (WIFSIGNALED(childstatus)) {
            debug_test("killed with -%d", WTERMSIG(childstatus));
        }
    }

    if (pid < 0) { // Forked child
        debug_test("in fork() %c", '\n');
    }

    return NULL;
}

void threadpool_add_test_icmp(threadpool_t* pool)
{
    size_t flags = 0;
    size_t res = EXIT_SUCCESS;
    struct timespec wait_for_ret = {.tv_sec = 1};
    struct timespec start = {0};
    struct timespec finish = {0};
    assert(timespec_get(&start, TIME_UTC));

    char* threadpool_task1_arg1 = "8.8.8.8";
    debug_test("will submit arg1 %s", threadpool_task1_arg1);

    composite_arg_t* arg1 = safe_alloc(sizeof (composite_arg_t));
    arg1->arg = safe_alloc(strlen(threadpool_task1_arg1) + 1);
    strncpy(arg1->arg, threadpool_task1_arg1, strlen(threadpool_task1_arg1));

    arg1->block_to_store_retval = safe_alloc(THREAD_RETVAL);

    threadpool_task_t task1 = {&threadpool_task_icmp, arg1};
    res = threadpool_add(pool, &task1, flags);
    assert(res == EXIT_SUCCESS);
    assert(pool->count == 1);

    assert(pool->queue[0].function == &threadpool_task_icmp);
    debug_test("will compare %s (%zu) with %s (%zu)",
            pool->queue[0].argument->arg,
            strlen(pool->queue[0].argument->arg),
            threadpool_task1_arg1,
            strlen(threadpool_task1_arg1));

    assert(strcmp(pool->queue[0].argument->arg, threadpool_task1_arg1) == 0);
    assert(strcmp(pool->queue[0].argument->arg, "fake"));

    nanosleep(&wait_for_ret, NULL);
    assert(strcmp(pool->queue[0].argument->block_to_store_retval, threadpool_task1_arg1) == 0);
    assert(strcmp(pool->queue[0].argument->block_to_store_retval, "fake"));

    assert(timespec_get(&finish, TIME_UTC));

    safe_free((void**) &(pool->queue[0].argument->block_to_store_retval));
    safe_free((void**) &arg1->arg);
    safe_free((void**) &arg1);

    size_t diff = finish.tv_nsec - start.tv_nsec;
    debug_test("add finished, pool->count %zu, time difference: %zu", pool->count, diff);
}

void* threadpool_task1(composite_arg_t* arg1)
{
    void* arg = arg1->arg;
    void* retval = arg1->block_to_store_retval;

    debug_test("arg %s, block to store retval %p", arg, retval);

    if (retval) {
        strncpy(retval, arg, strlen(arg));
    }

    return NULL;
}

void threadpool_add_test(threadpool_t* pool)
{
    size_t flags = 0;
    size_t res = EXIT_SUCCESS;
    struct timespec wait_for_ret = {.tv_sec = 1};

    char* threadpool_task1_arg1 = "success 1 <<<>>>\t";
    char* threadpool_task1_arg2 = "success 2 <<<>>>\t";
    debug_test("will submit arg1 %s, arg2 %s", threadpool_task1_arg1, threadpool_task1_arg2);

    composite_arg_t* arg1 = safe_alloc(sizeof (composite_arg_t));
    composite_arg_t* arg2 = safe_alloc(sizeof (composite_arg_t));
    arg1->arg = safe_alloc(strlen(threadpool_task1_arg1) + 1);
    arg2->arg = safe_alloc(strlen(threadpool_task1_arg2) + 1);
    strncpy(arg1->arg, threadpool_task1_arg1, strlen(threadpool_task1_arg1));
    strncpy(arg2->arg, threadpool_task1_arg2, strlen(threadpool_task1_arg2));

    arg1->block_to_store_retval = safe_alloc(THREAD_RETVAL);
    arg2->block_to_store_retval = safe_alloc(THREAD_RETVAL);

    assert(strcmp(threadpool_task1(arg1), "threadpool_task1 ok"));
    assert(strcmp(threadpool_task1(arg1), "threadpool_task1 ok\n") == 0);
    assert(strcmp(threadpool_task1(arg2), "threadpool_task1 ok\n") == 0);

    threadpool_task_t task1 = {&threadpool_task1, arg1};
    res = threadpool_add(pool, &task1, flags);
    assert(res == EXIT_SUCCESS);
    assert(pool->count == 1);

    assert(pool->queue[0].function == &threadpool_task1);
    debug_test("will compare %s (%zu) with %s (%zu)",
            pool->queue[0].argument->arg,
            strlen(pool->queue[0].argument->arg),
            threadpool_task1_arg1,
            strlen(threadpool_task1_arg1));

    assert(strcmp(pool->queue[0].argument->arg, threadpool_task1_arg1) == 0);
    assert(strcmp(pool->queue[0].argument->arg, "fake"));

    nanosleep(&wait_for_ret, NULL);
    assert(strcmp(pool->queue[0].argument->block_to_store_retval, threadpool_task1_arg1) == 0);
    assert(strcmp(pool->queue[0].argument->block_to_store_retval, "fake"));

    threadpool_task_t task2 = {&threadpool_task1, arg2};
    res = threadpool_add(pool, &task2, flags);
    assert(res == EXIT_SUCCESS);
    //    assert(pool->count == 2);

    assert(pool->queue[1].function == &threadpool_task1);
    assert(strcmp(pool->queue[1].argument->arg, threadpool_task1_arg2) == 0);
    assert(strcmp(pool->queue[1].argument->arg, pool->queue[0].argument->arg));

    nanosleep(&wait_for_ret, NULL);
    assert(strcmp(pool->queue[1].argument->block_to_store_retval, threadpool_task1_arg2) == 0);
    assert(strcmp(pool->queue[1].argument->block_to_store_retval, "fake"));


    safe_free((void**) &(pool->queue[0].argument->block_to_store_retval));
    assert(pool->queue[0].argument->block_to_store_retval == NULL);
    assert(arg1->block_to_store_retval == NULL);

    safe_free((void**) &(pool->queue[1].argument->block_to_store_retval));
    assert(pool->queue[1].argument->block_to_store_retval == NULL);
    assert(arg2->block_to_store_retval == NULL);

    safe_free((void**) &arg1->arg);
    assert(arg1->arg == NULL);

    safe_free((void**) &arg2->arg);
    assert(arg2->arg == NULL);

    safe_free((void**) &arg1);
    assert(arg1 == NULL);

    safe_free((void**) &arg2);
    assert(arg2 == NULL);

    debug_test("add finished, pool->count %zu", pool->count);
}

// threadpool_destroy()
void threadpool_destroy_test_graceful(threadpool_t* pool)
{
    size_t flags = threadpool_graceful;
    size_t res = EXIT_SUCCESS;

    assert(!pool->shutdown);

    res = threadpool_destroy(pool, flags);
    assert(res == EXIT_SUCCESS);
}

void threadpool_destroy_test_immediate(threadpool_t* pool)
{
    size_t flags = threadpool_immediate;
    size_t res = EXIT_SUCCESS;

    assert(!pool->shutdown);

    res = threadpool_destroy(pool, flags);
    assert(res == EXIT_SUCCESS);
}

// threadpool_free()
void threadpool_free_test(threadpool_t* pool)
{
    size_t res = EXIT_SUCCESS;

    pool->started = 0;

    res = threadpool_free(pool);
    assert(res == EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
    debug_info("Threadpool test starting... %c", '\n');

    debug_test("---------- test #1... %c", '\n');
    threadpool_t* testpool_destroy_immediate_noadd = threadpool_create_test();
    threadpool_destroy_test_immediate(testpool_destroy_immediate_noadd);

    debug_test("Cleaning up, track_block should not indicate any leftovers now... %c", '\0');
    if (!track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_FAILURE);
    }

    debug_test("---------- test #2... %c", '\n');
    threadpool_t* testpool_destroy_graceful_noadd = threadpool_create_test();
    threadpool_destroy_test_graceful(testpool_destroy_graceful_noadd);

    debug_test("Cleaning up, track_block should not indicate any leftovers now... %c", '\0');
    if (!track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_FAILURE);
    }

    debug_test("---------- test #3/icmp... %c", '\n');
    threadpool_t* testpool_thread_icmp = threadpool_create_test();
    threadpool_add_test_icmp(testpool_thread_icmp);
    threadpool_destroy_test_immediate(testpool_thread_icmp);

    debug_test("Cleaning up, track_block should not indicate any leftovers now... %c", '\0');
    if (!track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_FAILURE);
    }

    debug_test("---------- test #4... %c", '\n');
    threadpool_t* testpool_thread = threadpool_create_test();
    threadpool_add_test(testpool_thread);
    threadpool_destroy_test_graceful(testpool_thread);

    // Cleanup
    debug_test("Exiting, track_block should not indicate any leftovers now... %c", '\0');
    if (track_block(NULL, MODE_GLOBAL_CLEANUP_ON_SHUTDOWN)) {
        return (EXIT_SUCCESS);
    }
    debug_error("track_block failed! %c", '\0');
    return (EXIT_FAILURE);
}
