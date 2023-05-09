#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <pthread.h>

// Define a struct to represent a thread pool
typedef struct thread_pool
{
    int number_of_threads; // The number of worker threads in the pool
    pthread_t *threads_array; // An array to store the worker threads
    void (*destroy)(struct thread_pool *pool); // A function pointer to destroy the pool
    pthread_mutex_t mutex; // A mutex to protect shared data
} thread_pool_t;

// Define a struct to store the key and mode values
typedef struct {
    int key;
    int mode;
} encryption_args_t;

// Declare function prototypes for initializing, waiting for, and destroying the thread pool
void init_pool(thread_pool_t *pool);
void wait_and_destroy_pool(thread_pool_t *pool);
void destroy_pool(thread_pool_t *pool);

// Declare a function prototype for the worker thread function
void *worker_thread(void *arg);

// Declare a function prototype for encrypting or decrypting data
void encrypt_decrypt_data(char *data, int key, int mode);

// Define a default worker function that simply exits immediately
void *default_worker(void *arg) {
    pthread_exit(NULL);
}


#endif /* THREAD_POOL_H */