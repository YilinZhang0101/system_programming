/**
 * password_cracker
 * CS 341 - Fall 2023
 */
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>


typedef struct thread_data_t {
    pthread_t thread_id;
    size_t id;
    int computed_hashes;
} thread_data_t;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static size_t total_threads;

static char userName[16], password[16], knownpass[32];
static int task_pending;
static char* decrypted_password;
static long range_count;
static int overall_hash_count = 0;

pthread_barrier_t start_barrier, end_barrier;

char* generate_initial_password(thread_data_t* data) {
    int unknown_letter = strlen(knownpass) - getPrefixLength(knownpass);
    long start_pos;
    range_count = 0;
    getSubrange(unknown_letter, total_threads, data->id, &start_pos, &range_count);

    char *init_password = strdup(knownpass);
    setStringPosition(init_password + getPrefixLength(knownpass), start_pos);
    
    pthread_mutex_lock(&mut);
    v2_print_thread_start(data->id, userName, start_pos, init_password);
    pthread_mutex_unlock(&mut);

    return init_password;
}

int search_for_password(thread_data_t* data, char* initial_password) {
    struct crypt_data crypt_info;
    crypt_info.initialized = 0;

    int thread_status = 2;
    // 0 - the thread has successfully cracked the password
    // 1 - the thread was stopped early because the password was cracked by another thread
    // 2 - the thread has finished searching in the range it was given and failed to find the password
    char *hashed = NULL;

    long i = 0;
    while(i < range_count) {
        if (decrypted_password != NULL) {
            thread_status = 1;
            break;
        }
        
        hashed = crypt_r(initial_password, "xx", &crypt_info);
        if (strcmp(hashed, password) == 0) {
            decrypted_password = initial_password;
            thread_status = 0;
            break;
        }

        incrementString(initial_password);
        data->computed_hashes++;
        i++;
    }

    return thread_status;
}

void *password_cracker_thread(void *arg) {
    thread_data_t *data = arg;
    while (1) {
        pthread_barrier_wait(&start_barrier);
        if (task_pending == 0) {
            break;
        }
        char *init_password = generate_initial_password(data);
        
        int thread_status = search_for_password(data, init_password);

        if (thread_status == 1 || thread_status == 2) {
            free(init_password);
        }
        pthread_mutex_lock(&mut);
        v2_print_thread_result(data->id, data->computed_hashes, thread_status);
        pthread_mutex_unlock(&mut);
        
        pthread_barrier_wait(&end_barrier);
    }
    return NULL;
}

int start(size_t thread_count) {
    pthread_barrier_init(&start_barrier, NULL, thread_count + 1);
    pthread_barrier_init(&end_barrier, NULL, thread_count + 1);
    
    thread_data_t *thread_data_array = malloc(thread_count * sizeof(thread_data_t));
    total_threads = thread_count;
    for (size_t i = 0; i < thread_count; i ++) {
        thread_data_array[i].thread_id = 0;
        thread_data_array[i].id = i + 1;
        thread_data_array[i].computed_hashes = 0;
        pthread_create(&(thread_data_array[i].thread_id), NULL, password_cracker_thread, thread_data_array + i);  
    }
    
    char *command = NULL;
    size_t buffer_size = 0;
    
    while (getline(&command, &buffer_size, stdin) != -1) {
        task_pending = 1;
        sscanf(command, "%s %s %s\n", userName, password, knownpass);
        decrypted_password = NULL;
        
        pthread_mutex_lock(&mut);
        v2_print_start_user(userName);
        pthread_mutex_unlock(&mut);
        
        double timestamp_start = getTime();
        double cpu_timestamp_start = getCPUTime();
        
        pthread_barrier_wait(&start_barrier);
        pthread_barrier_wait(&end_barrier);
        
        size_t j = 0;
        while (j < thread_count) {
            overall_hash_count += thread_data_array[j].computed_hashes;
            j++;
        }

        pthread_mutex_lock(&mut);
        double timestamp_end = getTime();
        double cpu_timestamp_end = getCPUTime();
        v2_print_summary(userName, decrypted_password, overall_hash_count, timestamp_end - timestamp_start,
                        cpu_timestamp_end - cpu_timestamp_start, (!decrypted_password));
        pthread_mutex_unlock(&mut);
        
        free(decrypted_password);
    }
    
    task_pending = 0;
    pthread_barrier_wait(&start_barrier);
    
    for (size_t i = 0; i < thread_count;i ++) {
        pthread_join(thread_data_array[i].thread_id, NULL);
    }
    free(thread_data_array);
    free(command);

    pthread_barrier_destroy(&start_barrier);
    pthread_barrier_destroy(&end_barrier);
    return 0;
}
