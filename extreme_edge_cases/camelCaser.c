/**
 * extreme_edge_cases
 * CS 341 - Fall 2023
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
   
    if (!input_str) {
        return NULL;
    }
    int out_num = 0;
    const char *input_s1 = input_str;
    while(*input_s1) {       
        if(ispunct(*input_s1)) {
            out_num ++;
        }
        input_s1 ++;
    }
    out_num ++;

    char** result = malloc(out_num * sizeof(char*));
	result[out_num-1] = NULL;

    int i = 0, head = 0, idx = 0;
    const char *input_s2 = input_str;

    while (input_s2[i]) {
        if (ispunct(input_s2[i])) {
			// printf("%d ", i);
            char * result_e = malloc((i - head + 1) * sizeof(char));
            result_e[i - head] = '\0';

			int j = head, k = 0;
            int isFirst = 1;

			while (j < i) {
				// printf("%d ", i);
				// printf("%d ", j);
				while (j < i && isspace(input_s2[j])) {
					j ++;
				}
				// printf("%d\n", j);
				if(j < i){
					if(isFirst == 1) {
						result_e[k] = tolower(input_s2[j]);
						isFirst = 0;
					}
					else {
						result_e[k] = toupper(input_s2[j]);
					}
					j ++;
					k ++;

					while (j < i && !isspace(input_s2[j]) && !ispunct(input_s2[j])) {
						result_e[k++] = tolower(input_s2[j++]);
					}
				}
			}
			result_e[i - head] = '\0';
			result[idx++] = result_e;
			head = i + 1;
		}
		i ++;
    }
    //char* input_s2 = * input_str;
    return result;
}

void destroy(char **result) {
    // TODO: Implement me!
    if (result == NULL) return;
    for(int i = 0;result[i] != NULL;i ++) {
        free(result[i]);
    }
    free(result);
    return;
}
