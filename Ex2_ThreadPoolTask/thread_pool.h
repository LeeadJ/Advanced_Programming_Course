#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

// node for task queue linkedlist:
typedef struct task_node
{
    char *value;
    struct task_node *next;
} task_node_t;


// task queue struct for holding the tasks to execute:
typedef struct task_queue
{
    task_node_t *head;
    task_node_t *tail;
    pthread_mutex_t lock;   // added mutex lock for thread safety.
    pthread_cond_t not_empty;   // condition to signal the the queue is not empty.
} task_queue_t;


// Thread pool struct for managing threads and tasks
typedef struct thread_pool
{
    pthread_t *threads;    // array of thread IDs.
    int num_threads;
    task_queue_t *task_queue;   /// pointer to the task queue.
    pthread_mutex_t lock;
    pthread_cond_t not_empty; // signal that the queue is not empty.
    pthread_cond_t not_full;  // signal that the queue is not full.
    int shutdown;   // flag for thread pool shutdown.
    int key;    // encyption key.
    void (*func)(); //encryption function.
} thread_pool_t;


void task_queue_init(task_queue_t *queue);
void task_queue_push(task_queue_t *queue, task_node_t *node);
task_node_t *task_queue_pop(task_queue_t *queue);

void thread_pool_init(thread_pool_t *pool, char **argv);
void thread_pool_submit(thread_pool_t *pool, void (*func)(void *), void *arg);
void thread_pool_shutdown(thread_pool_t *pool);

#endif /* THREAD_POOL_H */