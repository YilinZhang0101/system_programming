/**
 * password_cracker
 * CS 341 - Fall 2023
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>

#include "../includes/queue.h"

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static size_t success_count = 0;
static size_t fail_count = 0;
static size_t queue_size = 0;
static queue* new_q;

void* do_funct(void* arg) {
    // queue* qu = ((void**)arg)[0];
    // int thread_id = *(int*)((void**)arg)[1];
    int thread_id = (long)arg;
    double time0 = getThreadCPUTime();
    // char* command;
    pthread_mutex_lock(&mut);

    while (queue_size != 0) {
        queue_size --;
        char* command = queue_pull(new_q);
        pthread_mutex_unlock(&mut);
        if (!command) {
            return 0;
        }

        char userName[16], password[16];
        char knownpass[32];

        sscanf(command, "%s %s %s", userName, password, knownpass);
        v1_print_thread_start(thread_id, userName);

        
        char* unknown_dot = knownpass + getPrefixLength(knownpass);
        setStringPosition(unknown_dot, 0);

        struct crypt_data cdata;
        cdata.initialized = 0;

        size_t hash_count = 1;
        char *hashed = NULL;
        int flag = 1;

        while (incrementString(unknown_dot) != 0) {
            hash_count++;
            hashed = crypt_r(knownpass, "xx", &cdata);
            if (strcmp(hashed, password) == 0) {
                //find match
                pthread_mutex_lock(&mut);
                success_count++;
                pthread_mutex_unlock(&mut);
                flag = 0;
                break;
            }
        }
        if (flag == 1) {
            pthread_mutex_lock(&mut);
            fail_count++;
            pthread_mutex_unlock(&mut);
        }

        double time1 = getThreadCPUTime();
        v1_print_thread_result(thread_id, userName, knownpass, hash_count, time1 - time0, flag);
        free(command);
        pthread_mutex_lock(&mut);
    }
    pthread_mutex_unlock(&mut);
    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    new_q = queue_create(-1);
    //If non-positive, the queue will never block upon a push
    char* buff = NULL;
    size_t cap = 0;
    while (getline(&buff, &cap, stdin) != -1) {
        if (buff != NULL) {
            if (buff[strlen(buff) - 1] == '\n') {
                buff[strlen(buff) - 1] = '\0';
            }
            char* copy = strdup(buff);
            queue_push(new_q, copy);
        }
        queue_size ++;
    }
    free(buff);

    pthread_t new_tid[thread_count];
    for (size_t i = 0;i < thread_count;i ++) {
        // pthread_t tid_num = new_tid + i;
        // void* arg[1] = {(void*)i + 1};
        pthread_create(new_tid + i, NULL, do_funct, (void*)i + 1);
    }
    for (size_t i = 0;i < thread_count;i ++) {
        pthread_join(new_tid[i], NULL);
    }

    v1_print_summary(success_count, fail_count);
    queue_destroy(new_q);
    pthread_mutex_destroy(&mut);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}