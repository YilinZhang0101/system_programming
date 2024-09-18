/**
 * parallel_make
 * CS 341 - Fall 2023
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"

#include "vector.h"
#include "queue.h"
#include "set.h"
#include "dictionary.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t g_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t target_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

static graph* parsed_graph;
static vector* all_vertex_dfs;  
static vector* target_list;
static set* visited_vertex;

//check whether the graph beginning at cur has a cycle
int cycle_detect(void* cur) {
    if (visited_vertex == NULL) {
        visited_vertex = shallow_set_create();
    }
    int flag = 0;
    if (set_contains(visited_vertex, cur) == 0) {
        set_add(visited_vertex, cur);
        vector* get_neighbors = graph_neighbors(parsed_graph, cur);
        for (size_t i = 0;i < vector_size(get_neighbors);i ++) {
            if (cycle_detect(vector_get(get_neighbors, i))) {
                flag = 1;
                break;
            }
        }
        vector_destroy(get_neighbors);
    }
    else {
        flag = 1;
    }
    if (visited_vertex != NULL) {
        set_destroy(visited_vertex);
        visited_vertex = NULL;
    }
    return flag;
}

//get the ordered rules from the descendants of a goal rule and store them in rules_queue (inefficient order, but correct to match the order for test10)
void get_all_vertex_dfs(void* cur) {
    if (visited_vertex == NULL) {
        visited_vertex = shallow_set_create();
    }
    vector* neighbors = graph_neighbors(parsed_graph, cur);
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        get_all_vertex_dfs(vector_get(neighbors, i));
        // DFS
    }

    if (set_contains(visited_vertex, cur) == 0) {
        set_add(visited_vertex, cur);
        vector_push_back(all_vertex_dfs, cur);
    }
    vector_destroy(neighbors);
}

int get_state(rule_t* get_rule) {
    int new_state = get_rule->state;
    return new_state;
}

void set_state(rule_t* get_rule, int new_state) {
    pthread_mutex_lock(&g_mut);
    get_rule->state = new_state;
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&g_mut);
}

// Helper function to check if any dependencies have failed or not finished yet.
int check_dependencies(vector* neighbors) {
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        void* cur_target = vector_get(neighbors, i);
        rule_t* pre_rule = (rule_t*)graph_get_vertex_value(parsed_graph, cur_target);
        if (get_state(pre_rule) == -1) {
            return -1;
            // Dependency failed.
        }
        else if (get_state(pre_rule) == 0) {
            // State of the rule. Defaults to 0
            return 0;
            // Dependencies not finished.
        }
    }
    return 1;
    // All dependencies are good.
}

// Helper function to check if target or its dependencies are files and compare their timestamps.
int check_file_dependencies(void* target, vector* neighbors) {
    if (access((char*)target, F_OK) == -1) {
        // F_OK: Test for the existence of the file.
        return 0;
    }
    
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        void* cur_target = vector_get(neighbors, i);
        if (access((char*)target, F_OK) == -1) {
            continue;
        }
        struct stat init_target_state, cur_target_state;
        if (stat((char*)target, &init_target_state) == -1) {
            return -1;
        }  
        if (stat((char*)cur_target, &cur_target_state) == -1) {
            return -1;
        } 
        if (difftime(cur_target_state.st_mtime, init_target_state.st_mtime) < 0) {
            return 1; 
            // If the target's last modification time is newer than its dependency's modification time, 
            // it returns 1 (meaning the target needs to be rebuilt).
        }
    }
    return 0; 
    // Neither target nor dependencies are files or they have the same timestamp.
}

int execute_commands(vector* exec_commands) {
    int flag = 1;
    for (size_t i = 0; i < vector_size(exec_commands); i++) {
        if (system((char*)vector_get(exec_commands, i)) != 0) {
        // If any command fails (returns non-zero), the function returns -1.
            flag = -1;
            return flag;
        }
    }
    return flag; 
    // All commands executed successfully.
}

void destroy_target(void* target, vector* get_neighbors) {
    pthread_mutex_unlock(&target_mut);
    free(target);
    vector_destroy(get_neighbors);
}

void* run() {
    while (1) {
        pthread_mutex_lock(&target_mut);
        
        if (vector_size(all_vertex_dfs) == 0) {
            pthread_mutex_unlock(&target_mut);
            break;
        }

        void* target = NULL;
        vector* get_neighbors = NULL;
        rule_t* get_rule = NULL;
        int get_state = 0;
        size_t init_size = vector_size(all_vertex_dfs);
        
        pthread_mutex_lock(&g_mut);

        size_t i = 0;
        while (i < init_size) {
            char* temp = vector_get(all_vertex_dfs, i);
            target = strdup(temp);
            get_rule = (rule_t*)graph_get_vertex_value(parsed_graph, target);
            get_neighbors = graph_neighbors(parsed_graph, target);
            
            get_state = check_dependencies(get_neighbors);

            if (get_state != 0) {
                vector_erase(all_vertex_dfs, i);
                break;
            }
            // Neither target nor dependencies are files or they have the same timestamp.
            free(target);
            vector_destroy(get_neighbors);
            i++;
        }

        pthread_mutex_unlock(&g_mut);
        
        if (i == init_size) {
            pthread_cond_wait(&cv, &target_mut);
            pthread_mutex_unlock(&target_mut);
            continue;
        }

        if (get_state == 1) {
            get_state = check_file_dependencies(target, get_neighbors);
            destroy_target(target, get_neighbors);

            if (get_state == 0) {
                get_state = execute_commands(get_rule->commands);
            }
            set_state(get_rule, get_state);
        }
        else {
            destroy_target(target, get_neighbors);
            set_state(get_rule, get_state);
        }
    }

    pthread_exit(NULL);
}

void vector_check(vector* target_list) {
    for (size_t i = 0;i < vector_size(target_list);i ++) {
        void* cur = vector_get(target_list, i);
        if (cycle_detect(cur) != 0) {
            print_cycle_failure((char*)cur);
            vector_erase(target_list, i);
            i --;
        }
        i ++;
    }
}
void destroy_all() {
    graph_destroy(parsed_graph);
    vector_destroy(all_vertex_dfs);
    vector_destroy(target_list);
    // set_destroy(visited_vertex);
    // free all the memory
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    pthread_t thread_id[num_threads];

    parsed_graph = parser_parse_makefile(makefile, targets);
    target_list = graph_neighbors(parsed_graph, "");
    // the first one is always ''  : {'target'}
    // get all the first line - the target we want
    all_vertex_dfs = string_vector_create();
    // find in other functions

    vector_check(target_list);
    // delete those targets that have cycles

    if (vector_size(target_list) == 0) {
        destroy_all();
        return 0;
    }

    for (size_t i = 0; i < vector_size(target_list); i++) {
        void* target = vector_get(target_list, i);
        get_all_vertex_dfs(target);
    }

    //try running the commands
    for (size_t i = 0; i < num_threads; i++) {
        pthread_create(thread_id + i, NULL, run, NULL);
    }
    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(thread_id[i], NULL);
    }

    destroy_all();
    set_destroy(visited_vertex);
    return 0;
}
