#ifndef __UTILS_H_
#define __UTILS_H_

#include "string_util.h"
#include "threads.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
typedef struct thread_pool thread_pool;

// Creates a thread pool and returns a pointer to it.
thread_pool *pool_init(int max_threads);

// Return maximum number of threads.
int pool_get_max_threads(thread_pool *);

// Insert task into thread pool. The pool will call work_routine and pass arg
// as an argument to it. This is a similar interface to pthread_create.
void pool_add_task(thread_pool *, void *(*work_routine)(void *), void *arg);

// Blocks until the thread pool is done executing its tasks.
void pool_wait(thread_pool *);

// Cleans up the thread pool, frees memory. Waits until work is done.
void pool_destroy(thread_pool *);

const int TASK_QUEUE_MAX = 2000;


struct task_data {
    void *(*work_routine)(void *);
    void *arg;
};


struct thread_pool {
    // N worker threads.
    pthread_t *worker_threads;

    // A circular queue that holds tasks that are yet to be executed.
    struct task_data* task_queue;

    // Head and tail of the queue.
    int queue_head, queue_tail;

    // How many worker threads can we have.
    int max_threads;

    // How many tasks are scheduled for execution. We use this so that
    // we can wait for completion.
    int scheduled;

    pthread_mutex_t mutex;

    // A condition that's signaled on when we go from a state of no work
    // to a state of work available.
    pthread_cond_t work_available;

    // A condition that's signaled on when we don't have any more tasks scheduled.
    pthread_cond_t done;
};


int pool_get_max_threads(struct thread_pool *pool)
{
    return pool->max_threads;
}


struct task_thread_args {
    struct thread_pool* pool;
    struct task_data td;
};


void *worker_thread_func(void *pool_arg)
{
    // DEB("[W] Starting work thread.");
    struct thread_pool *pool = (struct thread_pool *)pool_arg;

    while (1) {
        struct task_data picked_task;

        Pthread_mutex_lock(&pool->mutex);

        while (pool->queue_head == pool->queue_tail) {
            // DEB("[W] Empty queue. Waiting...");
            Pthread_cond_wait(&pool->work_available, &pool->mutex);
        }

        assert(pool->queue_head != pool->queue_tail);
        // DEBI("[W] Picked", pool->queue_head);
        picked_task = pool->task_queue[pool->queue_head % TASK_QUEUE_MAX];
        pool->queue_head++;

        // The task is scheduled.
        pool->scheduled++;

        Pthread_mutex_unlock(&pool->mutex);

        // Run the task.
        picked_task.work_routine(picked_task.arg);

        Pthread_mutex_lock(&pool->mutex);
        pool->scheduled--;

        if (pool->scheduled == 0) {
            Pthread_cond_signal(&pool->done);
        }
        Pthread_mutex_unlock(&pool->mutex);
    }
    return NULL;
}


void pool_add_task(struct thread_pool *pool, void *(*work_routine)(void*), void *arg)
{
    Pthread_mutex_lock(&pool->mutex);
    // DEB("[Q] Queueing one item.");
    if (pool->queue_head == pool->queue_tail) {
        Pthread_cond_broadcast(&pool->work_available);
    }

    struct task_data task;
    task.work_routine = work_routine;
    task.arg = arg;

    pool->task_queue[pool->queue_tail % TASK_QUEUE_MAX] = task;
    pool->queue_tail++;

    Pthread_mutex_unlock(&pool->mutex);
}


void pool_wait(struct thread_pool *pool)
{
    // DEB("[POOL] Waiting for completion.");
    Pthread_mutex_lock(&pool->mutex);
    while (pool->scheduled > 0) {
        Pthread_cond_wait(&pool->done, &pool->mutex);
    }
    Pthread_mutex_unlock(&pool->mutex);
    // DEB("[POOL] Waiting done.");
}


struct thread_pool* pool_init(int max_threads)
{
    struct thread_pool* pool = malloc(sizeof(struct thread_pool));

    pool->queue_head = pool->queue_tail = 0;
    pool->scheduled = 0;
    pool->task_queue = malloc(sizeof(struct task_data) * TASK_QUEUE_MAX);

    pool->max_threads = max_threads;
    pool->worker_threads = malloc(sizeof(pthread_t) * max_threads);

    Pthread_mutex_init(&pool->mutex);
    Pthread_cond_init(&pool->work_available);
    Pthread_cond_init(&pool->done);

    for (int i = 0; i < max_threads; i++) {
        Pthread_create(&pool->worker_threads[i], NULL, worker_thread_func, pool);
    }

    return pool;
}


void pool_destroy(struct thread_pool *pool)
{
    pool_wait(pool);

    for (int i = 0; i < pool->max_threads; i++) {
        Pthread_detach(pool->worker_threads[i]);
    }

    free(pool->worker_threads);
    free(pool->task_queue);

    free(pool);
}
void error(char *message)
{
    perror(message);
    exit(1);
}

char *read_text_from_socket(int sockfd)
{
    const int BUF_SIZE = 256;
    char *buffer = malloc(BUF_SIZE);

    char *result = malloc(1);
    result[0] = '\0';

    int n;
    while (1) {
        int n = read(sockfd, buffer, BUF_SIZE - 1);
        if (n < 0) {
            error("Error reading from socket");
        }
        buffer[n] = '\0';
        char *last_result = result;
        result = strappend(last_result, buffer);
        free(last_result);
        if (n < BUF_SIZE - 1) {
            break;
        }
    }

    free(buffer);

    return result;
}

void write_to_socket(int sockfd, const char *message)
{
    if (write(sockfd, message, strlen(message)) == -1) {
        error("write_to_socket");
    }

}

#endif
