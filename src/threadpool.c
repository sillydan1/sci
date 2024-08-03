#include "threadpool.h"
#include "log.h"
#include <stdlib.h>

static threadpool_work* threadpool_work_create(thread_func func, void *arg) {
    threadpool_work* result;
    if (func == NULL)
        return NULL;
    result = malloc(sizeof(threadpool_work));
    result->func = func;
    result->arg = arg;
    result->next = NULL;
    return result;
}

static void threadpool_work_destroy(threadpool_work* work) {
    if (work == NULL)
        return;
    free(work);
}

static threadpool_work* threadpool_work_pop(threadpool* pool) {
    threadpool_work* work;
    if (pool == NULL)
        return NULL;

    work = pool->work_first;
    if (work == NULL)
        return NULL; // TODO: This should propbably be using optionals

    if (work->next == NULL) {
        pool->work_first = NULL;
        pool->work_last = NULL;
        return work;
    }
    pool->work_first = work->next;
    return work;
}

static void* threadpool_worker(void *arg) {
    log_trace("threadpool worker spawned");
    threadpool* pool = arg;
    threadpool_work* work;
    while(true) {
        // Wait for work
        pthread_mutex_lock(&(pool->work_mutex));
        while (pool->work_first == NULL && !pool->stop)
            pthread_cond_wait(&(pool->work_cond), &(pool->work_mutex));
        if (pool->stop)
            break;
        work = threadpool_work_pop(pool);
        pool->working_count++;
        pthread_mutex_unlock(&(pool->work_mutex));

        // Do the work
        if (work != NULL) {
            work->func(work->arg);
            threadpool_work_destroy(work);
        }

        pthread_mutex_lock(&(pool->work_mutex));
        pool->working_count--;
        if (!pool->stop && pool->working_count == 0 && pool->work_first == NULL)
            pthread_cond_signal(&(pool->working_cond));
        pthread_mutex_unlock(&(pool->work_mutex));
    }
    pool->thread_count--;
    pthread_cond_signal(&(pool->working_cond));
    pthread_mutex_unlock(&(pool->work_mutex));
    return NULL;
}

threadpool* threadpool_create(size_t num) {
    log_trace("creating threadpool of size %d", num);
    threadpool* result;
    pthread_t thread;
    size_t i;
    if (num == 0)
        num = 2;
    result = calloc(1, sizeof(threadpool));
    result->thread_count = num;
    pthread_mutex_init(&(result->work_mutex), NULL);
    pthread_cond_init(&(result->work_cond), NULL);
    pthread_cond_init(&(result->working_cond), NULL);
    result->work_first = NULL;
    result->work_last  = NULL;
    for (i=0; i<num; i++) {
        pthread_create(&thread, NULL, threadpool_worker, result);
        pthread_setname_np(thread, "sci-worker");
        pthread_detach(thread);
    }
    return result;
}
// TODO: add log statements

void threadpool_destroy(threadpool* pool) {
    log_trace("destroying threadpool");
    threadpool_work* work;
    threadpool_work* work2;
    if (pool == NULL)
        return;
    pthread_mutex_lock(&(pool->work_mutex));
    work = pool->work_first;
    while (work != NULL) {
        work2 = work->next;
        threadpool_worker(work);
        work = work2;
    }
    pool->work_first = NULL;
    pool->stop = true;
    pthread_cond_broadcast(&(pool->work_cond));
    pthread_mutex_unlock(&(pool->work_mutex));

    threadpool_wait(pool);

    pthread_mutex_destroy(&(pool->work_mutex));
    pthread_cond_destroy(&(pool->work_cond));
    pthread_cond_destroy(&(pool->working_cond));

    free(pool);
}

bool threadpool_add_work(threadpool* pool, thread_func func, void *arg) {
    log_trace("adding work task to pool");
    threadpool_work* work;
    if (pool == NULL)
        return false;

    work = threadpool_work_create(func, arg);
    if (work == NULL)
        return false;

    pthread_mutex_lock(&(pool->work_mutex));
    if (pool->work_first == NULL) {
        pool->work_first = work;
        pool->work_last = pool->work_first;
    } else {
        pool->work_last->next = work;
        pool->work_last = work;
    }
    pthread_cond_broadcast(&(pool->work_cond));
    pthread_mutex_unlock(&(pool->work_mutex));

    return true;
}

void threadpool_wait(threadpool* pool) {
    if (pool == NULL)
        return;
    pthread_mutex_lock(&(pool->work_mutex));
    while (1) {
        if (pool->work_first != NULL || (!pool->stop && pool->working_count != 0) || (pool->stop && pool->thread_count != 0))
            pthread_cond_wait(&(pool->working_cond), &(pool->work_mutex));
        else
            break;
    }
    pthread_mutex_unlock(&(pool->work_mutex));
}
