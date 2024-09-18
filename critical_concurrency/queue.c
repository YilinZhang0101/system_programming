/**
 * critical_concurrency
 * CS 341 - Fall 2023
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    queue *new_q = malloc(sizeof(queue));
    if (new_q == NULL) {
        return NULL;
    }
    new_q->head = NULL;
    new_q->tail = NULL;
    new_q->size = 0;
    new_q->max_size = max_size;
    // if max_size <= 0, then decide in other functions
    pthread_mutex_init(&(new_q->m), NULL);
    pthread_cond_init(&(new_q->cv), NULL);

    return new_q;
}

void queue_destroy(queue *this) {
    /* Your code here */
    if (this == NULL) {
        return;
    }
    queue_node* it = NULL;
    for (it = this->head;it != NULL;) {
        this->head = it->next;
        free(it);
        it = this->head;
    }
    pthread_mutex_destroy(&(this->m));
    pthread_cond_destroy(&(this->cv));

    free(this);
}

void queue_push(queue *this, void *data) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));

    // If the queue has a max size and is full, wait
    while ((this->max_size) > 0 && (this->size) >= (this->max_size)) {
        pthread_cond_wait(&(this->cv), &(this->m));
    }

    queue_node* new_node = malloc(sizeof(queue_node));
    new_node->data = data;
    new_node->next = NULL;
    // only have next node

    if (this->tail) {
        this->tail->next = new_node;
    } else {
        this->head = new_node;
    }

    this->tail = new_node;
    (this->size) ++;

    pthread_cond_broadcast(&(this->cv));
    pthread_mutex_unlock(&(this->m));
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));

    while (this->head == NULL) {
        pthread_cond_wait(&(this->cv), &(this->m));
    }

    queue_node* get_pull_node = this->head;
    void* get_pull_data = get_pull_node->data;

    if (this->head->next == NULL) {
        this->head = NULL;
        this->tail = NULL;
    }
    else {
        this->head = this->head->next;
    }
    free(get_pull_node);
    (this->size) --;

    pthread_cond_broadcast(&(this->cv));
    pthread_mutex_unlock(&(this->m));

    return get_pull_data;
}
