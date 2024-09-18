/**
 * teaching_threads
 * CS 341 - Fall 2023
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct get_thread {
    reducer name_reduce;
    size_t length;
    int* list;
    int result;
    int base_case;
} get_thread;

void* start_routine(void* data);

/* You should create a start routine for your threads. */

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if (list == NULL) {
        return 0;
    }
    if (list_len < num_threads) {     
        // list is smaller than the number of threads, use fewer threads
        num_threads = list_len;
    }
    int num_each_list = list_len / num_threads;
    // if not divisible，just the most one
    get_thread* threat_list = malloc(sizeof(get_thread) * num_threads);
    pthread_t* pthread_list = malloc(sizeof(pthread_t) * num_threads);
    // give memory to pthread list, for creating
    
    // printf("yes3\n");
    for (size_t i = 0; i < num_threads; i ++) {
        if (i == num_threads - 1) {
            threat_list[i].length = list_len - num_each_list * (num_threads - 1);
        }
        else {
            threat_list[i].length = num_each_list;
        }
        threat_list[i].list = malloc(threat_list[i].length * sizeof(int));
        memcpy(threat_list[i].list, list + num_each_list * i, threat_list[i].length * sizeof(int));
        threat_list[i].name_reduce = reduce_func;
        threat_list[i].result = base_case;
        threat_list[i].base_case = base_case;
        // printf("yes %lu \n", i);

        pthread_create(pthread_list + i, NULL, start_routine, threat_list + i);
    }

    int ret = base_case;
    for (size_t j = 0; j < num_threads; j ++) {
        // wait for pthreads
        pthread_join(pthread_list[j], NULL);
        // all pthreads reduce
        ret = reduce_func(ret, threat_list[j].result);
        free(threat_list[j].list);
        threat_list[j].list = NULL;
    }

    free(threat_list);
    threat_list = NULL;
    free(pthread_list);
    pthread_list = NULL;
    // printf("%d\n", ret);

    return ret;
}

void* start_routine(void* data) {
    get_thread* this = (get_thread*) data;
    reducer name_reduce = this->name_reduce;
    size_t length = this->length;
    int* list = this->list;
    //int result = this->result;     
    int base_case = this->base_case;
    // printf("yes1\n");

    this->result = reduce(list, length, name_reduce, base_case);
    // printf("%d\n", this->result);
    return NULL;
}