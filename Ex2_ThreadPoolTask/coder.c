#include "codec.h"
#include "new.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Add a clock function to see if there is differences in time

void process_data(char *data, int key, int mode) {
    if (mode == 0) {
        encrypt(data, key);
    } else {
        decrypt(data, key);
    }
}

void *worker_thread2(void *arg) {
    int key = *((int *)arg);
    int mode = *((int *)arg + 1);
    char c;
    int counter = 0;
    char data[1024];

    while ((c = getchar()) != EOF) {
        pthread_mutex_lock(&mutex);
        data[counter] = c;
        counter++;

        if (counter == 1024) {
            process_data(data, key, mode);
            fwrite(data, 1, counter, stdout);
            counter = 0;
        }
        pthread_mutex_unlock(&mutex);
    }

    if (counter > 0) {
        process_data(data, key, mode);
        fwrite(data, 1, counter, stdout);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: coder key [-e | -d] < file\n");
        printf("!! data more than 1024 char will be ignored !!\n");
        return 0;
    }

    int key = atoi(argv[1]);
    int mode = (strcmp(argv[2], "-e") == 0) ? 0 : 1;

    int num_threads = 4; // You can adjust this based on your system's available cores
    ThreadPool tp;
    threadpool_init(&tp, num_threads);

    int args[2] = {key, mode};
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&tp.threads[i], NULL, worker_thread2, (void *)args);
    }

    threadpool_wait(&tp);
    threadpool_destroy(&tp);
    pthread_mutex_destroy(&mutex);

    return 0;
}