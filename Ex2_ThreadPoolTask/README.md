# Ex2: ThreadPool

### Table of Content
* [About The Assignment](#About-The-Assignment)
* [implementation](#Implementation)
* [Functions](#Functions)
* [How To Execute](#How-To-Execute)


---
## About This Assignment
In this assignment, we were asked to implement a threadpool tool to encrypt/decrypt data by using a shareed library containing the functions. The goal was to implement a threadpool and implement syncronization (as studied at class) by use of multi-threading and utilise multi-cores in C.

---

## Implementation
In this C program that a thread pool is implemented to encrypt or decrypt data using a key provided as a command-line argument. The program reads data from standard input, encrypts or decrypts the data, and writes the processed data to standard output. The program creates worker threads, each of which reads input characters from the standard input stream and processes them. The amount of worker threads are initialized based on the amount of proccessors or cores available to the operating system; `sysconf(_SC_NPROCESSORS_ONLN)`. The threadpool uses mutex thread locks to control the access worker threads have to shared resources. This is done to avoid problems when multipul threads are working at the same time.

---
## Functions
The program includes the following functions:

- `encrypt_decrypt_data(char *data, int key, int mode):` <br>
This function takes a character array, an integer key, and an integer mode. It uses the encrypt or decrypt function (defined in codec.h) to encrypt or decrypt the data using the provided key, depending on the value of mode. <br><br>
- `worker_thread(void *arg):`<br>
 This function is executed by worker threads. It takes a void pointer arg as an argument, which is expected to point to an encryption_args_t struct containing the encryption/decryption key and mode. The worker thread reads input characters from the standard input stream, processes them using encrypt_decrypt_data(), and writes the processed data to the standard output stream.<br><br>
- `init_pool(thread_pool_t *pool):`<br>
 This function initializes a thread pool pool. It determines the number of online processors and allocates memory for the threads array in the pool. It also initializes the mutex in the pool and creates threads in the pool, assigning them the default_worker() function.<br><br>
- `destroy_pool(thread_pool_t *pool):`<br>
 This function destroys the thread pool pool. It joins all the threads in the pool and frees memory allocated for the threads array.<br><br>
- `wait_and_destroy_pool(thread_pool_t *pool):`<br>
 This function waits for all threads in the pool to finish executing and then destroys the thread pool.<br><br>
- `main(int argc, char *argv[]):`<br>
 This function is the entry point of the program. It checks if the number of command-line arguments is correct and if the second argument is either "-d" or "-e". If the arguments are valid, it converts the first argument to an integer and stores it in the key variable. It creates an encryption_args_t struct and initializes its fields with the key and mode values. It allocates memory for the thread pool struct and initializes the pool. It creates threads and passes them the worker thread function and the argument array. Finally, it waits for all threads to finish executing and then destroys the thread pool and mutex.

---

## How To Execute

1) Compile and build the program by running the following in the terminal.
```sh
make all
```
2) For encryption run:
```sh
coder key -e < my_original_file > encripted_file
```
or

```sh
cat input_file | ./coder key -e > encrypted_file
```
3) For decryption run:
```sh
coder key -d < encripted_file > decripted_file
```
or

```sh
cat encripted_file | ./coder key -d > decripted_file
```