
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// Constants
#define NUM_READERS 5
#define NUM_READS 5
#define NUM_WRITERS 5
#define NUM_WRITES 5

// Variables
pthread_t r_tid[NUM_READERS];
pthread_t w_tid[NUM_WRITERS];
pthread_cond_t r_cond;
pthread_cond_t w_cond;
pthread_mutex_t mut;
int g = 0;
int r_current = 0;
int r_waiting = 0;

// Reader
void *reader(void *vargp)
{
    // Store the value argument passed to this thread
    int *id = (int *)vargp;

    for(int i = 0; i<NUM_READS; ++i){

        // Sleep for some time
        usleep(1000 * (random() % NUM_READERS + NUM_WRITERS));

        // Enter critical section
        pthread_mutex_lock(&mut);
            r_waiting++;
            while (r_current == -1){
                pthread_cond_wait(&r_cond, &mut);
            }
            r_waiting--;
            int current = r_current++;
        pthread_mutex_unlock(&mut);

        // Print the argument, static and global variables
        printf("Reader thread ID: %d, Global: %d, Readers: %d\n", *id, g, current);

        // Exit critical section
        pthread_mutex_lock(&mut);
            r_current--;
            if (r_current == 0) {
                pthread_cond_signal(&w_cond);
            }
        pthread_mutex_unlock(&mut);
    }

    return NULL;
}

// Writer
void *writer(void *vargp)
{
    // Store the value argument passed to this thread
    int *id = (int *)vargp;

    for(int i = 0; i < NUM_WRITES; ++i){

        // Sleep for some time
        usleep(1000 * (random() % NUM_READERS + NUM_WRITERS));

        // Enter critical section
        pthread_mutex_lock(&mut);
            while (r_current != 0) {
                pthread_cond_wait(&w_cond, &mut);
            }
            r_current = -1;
            int current = r_current;
            int prev = g;
            ++g;
        pthread_mutex_unlock(&mut);

        printf("Writer thread ID: %d, Previous: %d, New: %d, Readers: %d\n", *id, prev, g, current);

        // Exit critical section
        pthread_mutex_lock(&mut);
            r_current = 0;
            if (r_waiting > 0) {
                pthread_cond_broadcast(&r_cond);
            } else {
                pthread_cond_signal(&w_cond);
            }
        pthread_mutex_unlock(&mut);
    }

    return NULL;
}

int main()
{
    int i, rc;

    srandom((unsigned int)time(NULL));

    // Create mutex
    rc = pthread_mutex_init(&mut, NULL);

    // Initialize condition variable
    rc = pthread_cond_init(&w_cond, NULL);
    rc = pthread_cond_init(&r_cond, NULL);

    // Create three reader threads
    for (i = 0; i < NUM_READERS; i++){
        rc = pthread_create(&r_tid[i], NULL, writer, &w_tid[i]);
    }

    // Create three writer threads
    for (i = 0; i < NUM_WRITERS; i++){
        rc = pthread_create(&w_tid[i], NULL, reader, &r_tid[i]);
    }

    // Wait on readers to finish
	for(i = 0; i < NUM_READERS; i++) {
		pthread_join(r_tid[i], NULL);
	}

	// Wait on writers to finish
	for(i = 0; i < NUM_WRITERS; i++) {
		pthread_join(w_tid[i], NULL);
	}

    pthread_exit(NULL);
    return 0;
}
