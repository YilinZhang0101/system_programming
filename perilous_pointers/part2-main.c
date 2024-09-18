/**
 * perilous_pointers
 * CS 341 - Fall 2023
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
// int main() {
//     // your code here
//     return 0;
// }

int main() {
    // printf("== first_step ==\n");
    int a1 = 81;
    first_step(a1);

    
    // printf("== second_step ==\n");
    int a2 = 132;
    int *b2 = &a2;
    second_step(b2);

    
    // printf("== double_step ==\n");
    int a3 = 8942;
    int *b3 = &a3;
    int **c3 = &b3;
    double_step(c3);

    
    // printf("== strange_step ==\n");
    int value = 15;
    char *ptr1 = (char *)&value - 5;
    strange_step(ptr1);

    
    // printf("== empty_step ==\n");
    int empty_str[] = {1, 2, 3, 0};
    empty_step(empty_str);

    
    // printf("== two_step ==\n");
    char* s2 = "abcu";
    char* s = s2;
    two_step(s, s2);

    
    // printf("== three_step ==\n");
    char* first = malloc(10 * sizeof(char));
    char* second = first + 2;
    char* third = second + 2;
    three_step(first, second, third);
    free(first);

    
    // printf("== step_step_step ==\n");
    char* first1 = "abcdefghijklmn";
    char* second1 = "hijklmnopqrst";
    char* third1 = "opqrstuvwxyz";
    // printf("%d", second1[1]);
    step_step_step(first1, second1, third1);

    
    // printf("== it_may_be_odd ==\n");
    char *a4 = "a";
    int b4 = (int)*a4;
    it_may_be_odd(a4, b4);

    
    // printf("== tok_step ==\n");
    char str[] = "Hi,CS241,great";
    tok_step(str);

    
    //printf("== the_end ==\n");
    char orange[] = "1230";
    // printf("%d %c\n", orange[0], orange[0]);
    orange[0] = 1;
    // printf("%d & %co\n", orange[0], orange[0]);
    //printf("%s\n", (int *)orange);
    // printf("%d\n", *(int *)orange);
    the_end(&orange, &orange);

    return 0;
}
