#ifndef SCI_THREADPOOL_H
#define SCI_THREADPOOL_H
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
// Very inspired by: https://nachtimwald.com/2019/04/12/thread-pool-in-c/

// a work function
typedef void (*thread_func)(void *arg);

// linked list for work functions
typedef struct threadpool_work {
    thread_func func;
    void* arg;
    struct threadpool_work* next;
} threadpool_work;

// thread pool object
typedef struct {
    threadpool_work* work_first;
    threadpool_work* work_last;
    pthread_mutex_t work_mutex;
    pthread_cond_t work_cond;
    pthread_cond_t working_cond;
    size_t working_count;
    size_t thread_count;
    bool stop;
} threadpool;

// create a new threadpool instance with `num` amount of worker threads
threadpool* threadpool_create(size_t num);

// destroy a threadpool instance
void threadpool_destroy(threadpool* pool);

// add work to the threadpool
bool threadpool_add_work(threadpool* pool, thread_func func, void* arg);

// wait for the remaining work in the threadpool to complete
void threadpool_wait(threadpool* pool);

#endif
