#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "thread_pool.h"
#include "codec.h"

#define MAX_DATA_BLOCK 1024
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// function to encrypt/decrypt data using the given key
void encrypt_decrypt_data(char *data, int key, int mode);

// function to be executed by worker threads
void *worker_thread(void *arg);

// initializes the thread pool
void init_pool(thread_pool_t *pool);

// waits for all threads to finish and then destroys the thread pool
void wait_and_destroy_pool(thread_pool_t *pool);

// destroys the thread pool
void destroy_pool(thread_pool_t *pool);



int main(int argc, char *argv[])
{
    // Check if the number of arguments is correct, and if the second argument is either "-d" or "-e":
    if(argc != 3 || (!(strcmp(argv[2], "-d")) && !(strcmp(argv[2], "-e"))))
    {
          printf("--ERROR IN MAIN ARGUMENTS--");
          exit(EXIT_FAILURE);  
    }

    // Convert the first and second arguments to an integer and store it in the key variable
    int key = atoi(argv[1]);
    int mode=0;

    // If the second argument is "-d", set mode to 1
    if(!strcmp(argv[2], "-d"))
        mode = 1;

    // Create an encryption_args_t struct and initialize its fields
    encryption_args_t args = { .key = key, .mode = mode };

    // Allocate memory for the thread pool struct and initialize the pool
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    pool->destroy = &destroy_pool; // Set the destroy function pointer
    init_pool(pool);

    // Create threads and pass them the worker thread function and the argument array
    for(int i=0; i < pool->number_of_threads; i++)
    {
        pthread_create(&pool->threads_array[i], NULL, worker_thread, (void *)&args);
    }

    // Wait for all threads to finish executing and then destroy the thread pool and mutex
    wait_and_destroy_pool(pool);
    pthread_mutex_destroy(&mutex);

    return 0;
}



// initializes the thread pool
void init_pool(thread_pool_t *pool) 
{
    // gets the number of online processors
    int system_thread_num = sysconf(_SC_NPROCESSORS_ONLN);
    pool->number_of_threads = system_thread_num;

    // dynamically allocates memory for the threads_array in the pool
    pool->threads_array = calloc(system_thread_num, sizeof(pthread_t));
    if (pool->threads_array == NULL) 
    {
        printf("Error: failed to allocate memory for threads array\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the mutex in the pool
    if (pthread_mutex_init(&pool->mutex, NULL) != 0) 
    {
        printf("Error: failed to initialize mutex\n");
        exit(EXIT_FAILURE);
    }

    // creates the threads in the pool and assigns them the default_worker function
    for (int i = 0; i < system_thread_num; i++) 
    {
        if (pthread_create(&pool->threads_array[i], NULL, default_worker, (void*)pool) != 0) 
        {
            printf("Error: failed to create thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
}





void destroy_pool(thread_pool_t *pool) 
{
    for (int i = 0; i < pool->number_of_threads; i++) 
    {
        pthread_join(pool->threads_array[i], NULL);
    }

    free(pool->threads_array);
}



void wait_and_destroy_pool(thread_pool_t *pool) 
{
    destroy_pool(pool);
}



void encrypt_decrypt_data(char *data, int key, int mode)
{
    switch (mode) {
        case 0:
            encrypt(data, key);
            break;
        case 1:
            decrypt(data, key);
            break;
        default:
            printf("Error: mode variable mismatch");
            exit(EXIT_FAILURE);
            break;
    }
}




void *worker_thread(void *arg) 
{
    int key = *(int *)arg;
    int mode = *((int *)arg + 1);
    char c;
    int counter = 0;

    // Initializes a character array to hold the data block to be encrypted/decrypted
    char data_block[MAX_DATA_BLOCK];
    
    // The worker thread runs in an infinite loop, reading input characters from the standard
    // input stream and processing them
    while (1) 
    {
        // Acquires the mutex lock to prevent other threads from accessing the standard input stream
        pthread_mutex_lock(&mutex);
        c = getchar();

        // Releases the mutex lock
        pthread_mutex_unlock(&mutex);
        
        // If the end of file character is encountered, the loop is exited
        if (c == EOF) 
        {
            break;
        }
        
        data_block[counter] = c;
        counter++;

        // If the data block is full, it is encrypted/decrypted, written to the standard output
        // stream, and the counter is reset to zero
        if (counter == MAX_DATA_BLOCK) 
        {
            encrypt_decrypt_data(data_block, key, mode);
            fwrite(data_block, 1, counter, stdout);
            counter = 0;
        }
    }


    // If there are remaining characters in the data block that have not been processed,
    // the data block is encrypted/decrypted, written to the standard output stream,
    // and the thread exits
    if (counter > 0) 
    {
        encrypt_decrypt_data(data_block, key, mode);
        fwrite(data_block, 1, counter, stdout);
    }

    pthread_exit(NULL);
}