/**
 * sci - a simple ci system
   Copyright (C) 2024 Asger Gitz-Johansen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SCI_THREADPOOL_H
#define SCI_THREADPOOL_H
#include "optional.h"
#include <pthread.h>
#include <stdbool.h>

// a work function
typedef void (*thread_func)(void *arg);

// linked list for work functions
typedef struct threadpool_work {
    thread_func func;
    void* arg;
    struct threadpool_work* next;
} threadpool_work;
typedef optional_type(threadpool_work*) optional_threadpool_work;

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
