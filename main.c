
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// Let us create a global variable to change it in threads
pthread_t r_tid[5];
pthread_t w_tid[5];
pthread_cond_t w_cond;
pthread_mutex_t mut;
int g = 0;
int r_waiting = 0;

// Reader
void *reader(void *vargp)
{
    // Store the value argument passed to this thread
    int *id = (int *)vargp;

    // Add to readers waiting
    ++r_waiting;

    // Sleep for some time
    sleep(rand() % 10);

    // Acquire mutex
    pthread_mutex_lock(&mut);

    // Print the argument, static and global variables
    printf("Reader thread ID: %d, Current: %d\n", *id, g);

    // Unlock mutex
    pthread_mutex_unlock(&mut);

    // Remove from readers waiting
    --r_waiting;

    // Signal writer availability condition
    if (r_waiting == 0) {
        pthread_cond_signal(&w_cond);
    }

    return NULL;
}

// Writer
void *writer(void *vargp)
{
    // Store the value argument passed to this thread
    int *id = (int *)vargp;

    // Sleep for some time
    sleep(rand() % 2);

    // Acquire mutex
    pthread_mutex_lock(&mut);

        // Check to see that no readers waiting
        while (r_waiting > 0) {
            pthread_cond_wait(&w_cond, &mut);
        }

        // Print the argument, static and global variables
        int prev = g;
        ++g;
        printf("Writer thread ID: %d, Previous: %d, New: %d\n", *id, prev, g);

    // Unlock mutex
    pthread_mutex_unlock(&mut);

    return NULL;
}

int main()
{
    int i, rc;

    // Create mutex
    rc = pthread_mutex_init(&mut, NULL);
    if (rc == -1){
        perror("error in pthread_mutex_init");
        exit(1);
    }

    // Initialize condition variable
    rc = pthread_cond_init(&w_cond, NULL);
    if (rc == -1){
        perror("error in pthread_cond_init");
        exit(1);
    }

    // Create three writer threads
    for (i = 0; i < 3; i++){
        rc = pthread_create(&w_tid[i], NULL, writer, &w_tid[i]);
        if (rc == -1) {
            perror("error in pthread_create");
            exit(1);
        }
    }

    // Create three reader threads
    for (i = 0; i < 3; i++){
        rc = pthread_create(&r_tid[i], NULL, reader, &r_tid[i]);
        if (rc == -1) {
            perror("error in pthread_create");
            exit(1);
        }
    }

    // Destroy lock
    pthread_mutex_destroy(&mut);

    pthread_exit(NULL);
    return 0;
}
