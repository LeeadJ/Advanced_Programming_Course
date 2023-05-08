#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <pthread.h>

typedef struct Node 
{
    char *data;
    struct Node *next;
} Node_t;


typedef struct thread_pool
{
    int number_of_threads;
    pthread_t *threads_array;
    void (*destroy)(struct thread_pool *pool);
    pthread_mutex_t mutex;
} thread_pool_t;

void init_pool(thread_pool_t *pool);
void wait_and_destroy_pool(thread_pool_t *pool);
void destroy_pool(thread_pool_t *pool);
void *worker_thread(void *arg);
void encrypt_decrypt_data(char *data, int key, int mode);

void *default_worker(void *arg) {
    pthread_exit(NULL);
}


#endif /* THREAD_POOL_H */