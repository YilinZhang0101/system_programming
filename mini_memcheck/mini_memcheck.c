/**
 * mini_memcheck
 * CS 341 - Fall 2023
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>

meta_data* head;
size_t total_memory_requested;
size_t total_memory_freed;
size_t invalid_addresses;
// the variable or function is declared, 
// but not defined, within this file (translation unit). 
// The actual definition resides in another file.

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    if (request_size == 0) {
        return NULL;
    }
    size_t meta_data_size = sizeof(size_t) + sizeof(char*) + sizeof(void*) + sizeof(meta_data*);
    meta_data* new_meta = malloc(request_size + meta_data_size);
    if (!new_meta) {
        return NULL;
    }
    new_meta->request_size = request_size;
    new_meta->filename = filename;
    new_meta->instruction = instruction;

    if (head != NULL) {
        new_meta->next = head;
        head = new_meta;
    } 
    else {
        new_meta->next = NULL;
        head = new_meta;
    }
    total_memory_requested += new_meta->request_size;
    return (void*) (new_meta + 1);
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    size_t mul_request_size = num_elements * element_size;
    void* mul_ret = mini_malloc(mul_request_size, filename, instruction);
    // add each element size together
    if (mul_ret != NULL) {
        memset(mul_ret, 0, mul_request_size);
        // zeros out the allocated memory
        return mul_ret;
    }
    return NULL;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if (payload == NULL) {
        return mini_malloc(request_size, filename, instruction);
    }
    if (request_size == 0) {
        mini_free(payload);
        return NULL;
    }

    meta_data* meta_to_be_realloc = (meta_data*)payload - 1;
    // find the meta_data of payload
    meta_data* it = head;
    meta_data* before = NULL;

    while (it != NULL) {
        if (it != meta_to_be_realloc) {
            before = it;
            it = it->next;
            continue;
            // continue finding
        }
        else {   // free it  
            size_t old_size = it->request_size;
            size_t meta_data_size = sizeof(size_t) + sizeof(char*) + sizeof(void*) + sizeof(meta_data*);
            meta_data* re_meta = realloc(it, meta_data_size + request_size);
            if (!re_meta) {
                return NULL;
            }
            re_meta->request_size = request_size;
            re_meta->filename = filename;
            re_meta->instruction = instruction;

            if (before == NULL) {
                head = re_meta;
                // the first one change
            }
            else {
                before->next = re_meta;
            }

            if (old_size >= request_size) {
                total_memory_freed += old_size - request_size;
            }
            else {
                total_memory_requested += request_size - old_size;
            }
            
            return (void*)(it + 1);
        }
    }
    invalid_addresses ++;
    return NULL;
}

void mini_free(void *payload) {
    // your code here
    if (payload == NULL) {
        return;
    }
    meta_data* meta_to_be_free = (meta_data*)payload - 1;
    // find the meta_data of payload
    meta_data* it = head;
    meta_data* before = NULL;

    while (it != NULL) {
        if (it != meta_to_be_free) {
            before = it;
            it = it->next;
            continue;
            // continue finding
        }
        else {   // free it        
            if (before == NULL) {
                head = it->next;
                // the first one change
            }
            else {
                before->next = it->next;
            }
            total_memory_freed += it->request_size;
            free(it);
            
            return;
        }
    }
    invalid_addresses ++;
    // if double free, also can't find it in mini_memcheck
    return;
}
