#include "codec.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "thread_pool.h"

#define MAX_DATA_SIZE 1024

void task_queue_init(task_queue_t *queue);
void task_queue_push(task_queue_t *queue, task_node_t *task_node);
task_node_t* task_queue_pop(task_queue_t *queue);
void thread_pool_init(thread_pool_t *pool, char **argv); 
void thread_pool_shutdown(thread_pool_t *pool); 
void *worker_thread(void *arg); 


int main(int argc, char *argv[])
{

    if(argc != 3 || (!(strcmp(argv[2], "-d")) && !(strcmp(argv[2], "-e"))))
    {
          printf("--ERROR IN MAIN ARGUMENTS--");
          exit(EXIT_FAILURE);  
    }
    char c;
    char *data = (char*)malloc(sizeof(char) * MAX_DATA_SIZE);
    int counter=0;
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    // init the task pool.
    thread_pool_init(pool, argv);

    while((c = getchar()) != EOF)
    {
        data[counter] = c;
        counter++;

        // When reaching max size, creat a new task and add to the queue.
        if(counter == MAX_DATA_SIZE)
        {
            task_node_t *new_task = malloc(sizeof(task_node_t));
            new_task->value = malloc(sizeof(char) * strlen(data) + 1);
            strcpy(new_task->value, data);
            new_task->next = NULL;
            task_queue_push(pool->task_queue, new_task);
            counter = 0;
        }
    }

    // if reached eEOF but data still is not empty, create new task"
    if(counter > 0)
    {
        task_node_t *new_task = malloc(sizeof(task_node_t));
        new_task->value = malloc(sizeof(char) * strlen(data) + 1);
        strcpy(new_task->value, data);
        new_task->next = NULL;
        task_queue_push(pool->task_queue, new_task);
    }

    while(pool->task_queue->head != NULL);
    thread_pool_shutdown(pool);

    exit(EXIT_SUCCESS);
}





/* Initializing the task queue. */
void task_queue_init(task_queue_t *queue) 
{
    queue->head = NULL;
    queue->tail = NULL;
    // init mutex lock to ensure only one thread can access the task queue.
    pthread_mutex_init(&queue->lock, NULL);
    //init not empty signal condition so worker threads can wait for tasks to be added.
    pthread_cond_init(&queue->not_empty, NULL);
}


/* Function to add task to task_queue. */
void task_queue_push(task_queue_t *queue, task_node_t *task_node) 
{
    // set a lock on the queue so no other thread can modify the queue.
    pthread_mutex_lock(&queue->lock);
    if (queue->tail == NULL) //queue is empty
    {
        queue->head = task_node;
        queue->tail = task_node;
    } 
    else // queue is not empty
    {
        queue->tail->next = task_node;
        queue->tail = task_node;
    }
    // set not empty signal to wake up any threads waiting to get a task.
    pthread_cond_signal(&queue->not_empty);
    // unlock the queue to allow other threads access.
    pthread_mutex_unlock(&queue->lock);
}

/* Function to remove task from queue. */
task_node_t* task_queue_pop(task_queue_t *queue) 
{
    if(queue->head == NULL) 
        return NULL;
    
    // lock the queue.
    pthread_mutex_lock(&queue->lock);
    task_node_t *task_node = queue->head;
    if (queue->head == queue->tail) 
    {
        queue->head = NULL;
        queue->tail = NULL;
    } 

    else 
        queue->head = queue->head->next;

    pthread_mutex_unlock(&queue->lock);
    return task_node;
}


/* Function to init the thread pool. */
void thread_pool_init(thread_pool_t *pool, char **argv) 
{
    // init num of threads to num of available CPUs on the system.
    pool->num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    pool->threads = malloc(sizeof(pthread_t) * pool->num_threads);
    pool->task_queue = malloc(sizeof(task_queue_t));
    // init the task.
    task_queue_init(pool->task_queue);
    // init the mutex lock.
    pthread_mutex_init(&pool->lock, NULL);
    //init not empty condition.
    pthread_cond_init(&pool->not_empty, NULL);
    // init not full condition.
    pthread_cond_init(&pool->not_full, NULL);
    // set encyption key
    pool->key = atoi(argv[1]);
    // set shutdown.
    pool->shutdown = 0;

    // set encyption/decryption functions:
    char *flag = argv[2];
    if(!strcmp(flag, "-e")) 
        pool->func = encrypt;
    else 
        pool->func = decrypt;
    
    // Creating the worker threads:
    for (int i = 0; i < pool->num_threads; i++) 
    {
        if(pthread_create(&pool->threads[i], NULL, worker_thread, (void*)pool))
        {
            printf("--ERROR WHILE CREATING THREAD\n");
            exit(EXIT_FAILURE);
        }
    }
}

/* Function to set the shutdown flag to the pool to 1. Signals all threads
   waiting on the "not_empty" condition to wake up.*/
void thread_pool_shutdown(thread_pool_t *pool) 
{
    // lock the pool for critical code.
    pthread_mutex_lock(&pool->lock);
    pool->shutdown = 1;
    // send signal to all "not_empty" condition waiting threads the pool is shutting down.
    pthread_cond_broadcast(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);

    // join the workerthreads.
    for (int i = 0; i < pool->num_threads; i++) 
    {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    free(pool->task_queue);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->not_empty);
    pthread_cond_destroy(&pool->not_full);
}


/* Function definition for the worker thread. */
void *worker_thread(void *arg) 
{
    // casting arg to thread_pool_t pointer, to allow worker thread to access the pool's info.
    thread_pool_t *pool = (thread_pool_t *) arg;
    while (1) 
    {
        // get next task from the task_queue.
        task_node_t *task_node = task_queue_pop(pool->task_queue);
        // If task is NULL, threadpool has been shutdown.
        if (task_node == NULL) 
            pthread_exit(NULL);
        
        pool->func(task_node->value, pool->key);
        printf("%s", task_node->value);
        free(task_node);
        
    }
    return NULL;
}

