/**
 * vector
 * CS 341 - Fall 2023
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    char* s;
    size_t length;
};

//self function
size_t sstring_length(sstring *this) {
    assert(this);
    return this->length;
}

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    sstring* ss = malloc(sizeof(sstring));
    char* new_s = malloc(strlen(input) + 1);
    strcpy(new_s, input);
    ss->s = new_s;
    ss->length = strlen(input);
    return ss;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    char *temp = malloc(input->length + 1);
    strcpy(temp, input->s);
    return temp;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    this->length = this->length + addition->length;
    this->s = realloc(this->s, this->length + 1);
    strcat(this->s, addition->s);
    return this->length;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    vector* ret = string_vector_create();
    size_t start = 0;
    for (size_t i = 0;i < this->length;i ++) {
        if (this->s[i] == delimiter) {
            char* w = malloc((i - start) * sizeof(char));
            strncpy(w, this->s + start, i - start);
            w[i - start] = '\0';
            vector_push_back(ret, w);
            start = i + 1;
        }
    }
    size_t last_len = this->length - start;
    char* w = malloc((last_len) * sizeof(char));
    strncpy(w, this->s + start, last_len);
    w[last_len] = '\0';
    vector_push_back(ret, w);

    return ret;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    if (!this || !target || !substitution) {
        return -1;
    }
    
    char *found_target = strstr(this->s + offset, target);
    if (!found_target) {
        return -1;
    }

    size_t len_before_target = found_target - this->s;
    size_t len_target = strlen(target);
    size_t len_substitution = strlen(substitution);
    size_t new_length = this->length - len_target + len_substitution;

    char *new_string = malloc(new_length + 1);

    strncpy(new_string, this->s, len_before_target);  // Part before target
    strncpy(new_string + len_before_target, substitution, len_substitution);  // Substitution
    strcpy(new_string + len_before_target + len_substitution, found_target + len_target);  // Part after target

    free(this->s);
    this->s = new_string;
    this->length = new_length;

    return 0;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    char* res = malloc((end - start + 1) * sizeof(char));
    res[end - start] = '\0';
    strncpy(res, this->s + start, end - start);
    return res;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    free(this->s);
    free(this);
    // this->NULL;
}

