/**
 * deadlock_demolition
 * CS 341 - Fall 2023
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
struct drm_t {
    pthread_mutex_t mut;
};

static graph *g;
static set* visited_vertex = NULL;

drm_t *drm_init() {
    /* Your code here */
    drm_t* new_d = malloc(sizeof(drm_t));
    if (new_d == NULL) {
        return NULL;
    }
    pthread_mutex_init(&(new_d->mut), NULL);
    pthread_mutex_lock(&mut);

    if (g == NULL) {
        g = shallow_graph_create();
    }
    graph_add_vertex(g, new_d);
    pthread_mutex_unlock(&mut);

    return new_d;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&mut);
    if (!graph_contains_vertex(g, thread_id) || !graph_contains_vertex(g, drm)) {
        pthread_mutex_unlock(&mut);
        return 0;
    }
    if (graph_adjacent(g, drm, thread_id)) {
        graph_remove_edge(g, drm, thread_id);
        pthread_mutex_unlock(&(drm->mut));
        pthread_mutex_unlock(&mut);
        return 1;
    }
    pthread_mutex_unlock(&mut);
    return 0;
}

int has_cycle(void* vertex) {
    if (visited_vertex == NULL) {
        visited_vertex = shallow_set_create();
    }
    int flag = 0;
    if (set_contains(visited_vertex, vertex) == 0) {
        set_add(visited_vertex, vertex);
        vector* get_neighbors = graph_neighbors(g, vertex);
        for (size_t i = 0;i < vector_size(get_neighbors);i ++) {
            void* new_ver = vector_get(get_neighbors, i);
            if (has_cycle(new_ver)) {
                return 1;
            }
        }
        vector_destroy(get_neighbors);
    }
    else {
        flag = 1;
    }
    set_destroy(visited_vertex);
    visited_vertex = NULL;
    return flag;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&mut);
    int isContained_thread = graph_contains_vertex(g, thread_id);
    if (isContained_thread == 0) {
        graph_add_vertex(g, thread_id);
    }
    if (graph_contains_vertex(g, drm)) {
        if (graph_adjacent(g, drm, thread_id)) {
            pthread_mutex_unlock(&mut);
            return 0;
        }
    }
    graph_add_edge(g, thread_id, drm);

    if (has_cycle(thread_id) == 0) {
        pthread_mutex_unlock(&mut);
        pthread_mutex_lock(&(drm->mut));
        pthread_mutex_lock(&mut);

        graph_remove_edge(g, thread_id, drm);
        graph_add_edge(g, drm, thread_id);
        
        pthread_mutex_unlock(&mut);
        return 1;
    }
    graph_remove_edge(g, thread_id, drm);
    pthread_mutex_unlock(&mut);

    return 0;
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    pthread_mutex_lock(&mut);
    graph_remove_vertex(g, drm);
    pthread_mutex_unlock(&mut);

    pthread_mutex_destroy(&(drm->mut));
    pthread_mutex_destroy(&mut);
    free(drm);
    return;
}
