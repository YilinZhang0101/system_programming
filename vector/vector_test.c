/**
 * vector
 * CS 341 - Fall 2023
 */
#include "vector.h"
#include <stdio.h>


void print_vector(char* description, vector *new_v){
    puts(description);
    char **begin = (char **)vector_begin(new_v);
    char **end = (char **)vector_end(new_v);
    while (begin < end) {
        printf("new_v[%ld]: %s\n", begin - (char **)vector_begin(new_v), *begin);
        begin++;
    }
    printf("size of vector: %zu\n", vector_size(new_v));
    printf("capacity of vector: %zu\n", vector_capacity(new_v));
    printf("vector is empty: %d\n", vector_empty(new_v));
    puts("*****************************");
}

int main(int argc, char *argv[]) {
    // Write your test cases here
    vector *new_v = vector_create(string_copy_constructor, string_destructor, string_default_constructor);

    print_vector("resize empty vector to 0:", new_v);

    vector_push_back(new_v, "elem0");    
    vector_push_back(new_v, "elem1");
    vector_push_back(new_v, "elem2");
    vector_push_back(new_v, "elem3");
    vector_push_back(new_v, "elem4");
    vector_push_back(new_v, "elem5");
    vector_push_back(new_v, "elem6");
    
    print_vector("Before modify:", new_v);

    vector_resize(new_v, 0);
    print_vector("after resize to 0", new_v);

    vector_resize(new_v, 11);
    print_vector("after resize to 11:", new_v);

    vector_resize(new_v, 20);
    print_vector("after resize to 20", new_v);

    vector_insert(new_v, 2, "NEW ELEMENT 2");
    print_vector("after insert at 2:", new_v);

    vector_set(new_v, 4, "elem four");
    print_vector("after set at 4:", new_v);

    vector_insert(new_v, vector_size(new_v), "elem7");
    print_vector("Edge case-insert at last index:", new_v);

    vector_erase(new_v, 0);
    print_vector("after erase at 0:", new_v);

    vector_erase(new_v, vector_size(new_v) - 1);
    print_vector("Edge case-erase at last index:", new_v);

    vector_resize(new_v, 16);
    print_vector("after resize to 16:", new_v);

    vector_resize(new_v, 7);
    print_vector("after resize to 7:", new_v);

    vector_resize(new_v, 17);
    print_vector("after resize to 17:", new_v);

    vector_reserve(new_v, 5);
    print_vector("after reserve size 5:", new_v);

    vector_reserve(new_v, 17);
    print_vector("after reserve size 17:", new_v);

    vector_reserve(new_v, 33);
    print_vector("after reserve size 33:", new_v);

    vector_clear(new_v);
    print_vector("after clear:", new_v);

    printf("begin address: %p\n", vector_begin(new_v));
    printf("end address: %p\n", vector_end(new_v));
    printf("front address: %p\n", vector_front(new_v));
    printf("back address: %p\n", vector_back(new_v));

    vector_destroy(new_v);
    return 0;
}

