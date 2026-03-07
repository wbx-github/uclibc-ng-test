#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

void *thread_func(void *arg)
{
    printf("Thread %ld:\n", (long)arg);
    printf("  PID        = %d\n", getpid());
    printf("  TID        = %d (gettid)\n", gettid());
    printf("  pthread ID = %lu\n\n", pthread_self());
    return NULL;
}

int main(void)
{
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];

    printf("Main thread:\n");
    printf("  PID        = %d\n", getpid());
    printf("  TID        = %d (gettid)\n", gettid());
    printf("  pthread ID = %lu\n\n", pthread_self());

    for (long i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, (void *)i) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

