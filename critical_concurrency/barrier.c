/**
 * critical_concurrency
 * CS 341 - Fall 2023
 */
#include "barrier.h"

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    int error = 0;
    error = pthread_mutex_destroy(&(barrier->mtx));
    if (error) {
        return error;
    }
    error = pthread_cond_destroy(&(barrier->cv));
    if (error) {
        return error;
    }
    return error;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    int error = 0;
    error = pthread_mutex_init(&(barrier->mtx), NULL);
    if (error) {
        return error;
    }
    error = pthread_cond_init(&(barrier->cv), NULL);
    if (error) {
        return error;
    }
    barrier->n_threads = num_threads;
    barrier->count = 0;
    barrier->times_used = 0;

    return error;
}

int barrier_wait(barrier_t *barrier) {
    pthread_mutex_lock(&(barrier->mtx));
    (barrier->count) ++;
    // how many threads reach the barrier
    
    if ((barrier->count) < (barrier->n_threads)) {
        pthread_cond_wait(&(barrier->cv), &(barrier->mtx));
    } 
    else {
        barrier->count = 0;
        (barrier->times_used) ++;
        pthread_cond_broadcast(&(barrier->cv));
    }

    pthread_mutex_unlock(&(barrier->mtx));
    return 0;
}
