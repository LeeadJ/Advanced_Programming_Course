#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "thread_pool2.h"
#include "codec.h"

#define MAX_DATA_BLOCK 1024
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// void encrypt_decrypt_data(char *data, int key, int mode);
// void *worker_thread(void *arg);
// void init_pool(thread_pool_t *pool);
// void wait_and_destroy_pool(thread_pool_t *pool);
// void destroy_pool(thread_pool_t *pool);



void init_pool(thread_pool_t *pool) 
{
    int system_thread_num = sysconf(_SC_NPROCESSORS_ONLN);
    pool->number_of_threads = system_thread_num;
    pool->threads_array = malloc(sizeof(pthread_t) * system_thread_num);
    pthread_mutex_init(&pool->mutex, NULL);
    for (int i = 0; i < system_thread_num; i++) 
    {
        pthread_create(&pool->threads_array[i], NULL, default_worker, NULL);
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
    // (mode == 0) ? encrypt(data, key) : decrypt(data, key);
    if (mode == 0) {
        encrypt(data, key);
    } else {
        decrypt(data, key);
    }
}



void *worker_thread(void *arg) {
    int key = *((int *)arg);
    int mode = *((int *)arg + 1);
    char c;
    int counter = 0;
    char data_block[MAX_DATA_BLOCK];

    while ((c = getchar()) != EOF) {
        pthread_mutex_lock(&mutex);
        data_block[counter] = c;
        counter++;

        if (counter == MAX_DATA_BLOCK) {
            encrypt_decrypt_data(data_block, key, mode);
            fwrite(data_block, 1, counter, stdout);
            counter = 0;
        }
        pthread_mutex_unlock(&mutex);
    }

    if (counter > 0) {
        encrypt_decrypt_data(data_block, key, mode);
        fwrite(data_block, 1, counter, stdout);
    }

    pthread_exit(NULL);
}







int main(int argc, char *argv[])
{
    if(argc != 3 || (!(strcmp(argv[2], "-d")) && !(strcmp(argv[2], "-e"))))
    {
          printf("--ERROR IN MAIN ARGUMENTS--");
          exit(EXIT_FAILURE);  
    }

    int key = atoi(argv[1]);
    int mode=0;
    if(!strcmp(argv[2], "-d"))
        mode = 1;
    
    int arg[2] = {key, mode};
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    init_pool(pool);

    for(int i=0; i < pool->number_of_threads; i++)
    {
        pthread_create(&pool->threads_array[i], NULL, worker_thread, (void *)arg);
    }

    wait_and_destroy_pool(pool);
    pthread_mutex_destroy(&mutex);

    return 0;
}

