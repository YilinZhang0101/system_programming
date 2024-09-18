/**
 * extreme_edge_cases
 * CS 341 - Fall 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!
    char** result = NULL;

    // case1: basic case
    result = camelCaser(" A2c sed 123   iS a WORD? No! disapeear ");
    // printf("%s", *result);
    // printf("%s", *(result+1));

    if (strcmp(result[0], "a2cSed123IsAWord")) {
        return 0;
    }
    if ( strcmp(result[1], "no")) {
        return 0;
    }
    destroy(result);

    // case2: NULL case
    result = camelCaser(NULL);
    // printf("%s", *result); // seg fault

    if (result) {
        return 0;
    }
    destroy(result);

    // case3: "" case
    result = camelCaser("");

    if (result[0] != 0) {
        return 0;
    }
    destroy(result);


    // case4: basic case
    result = camelCaser("hello. welcome to cs241?");
    // printf("%s\n", *result);

    if (strcmp(result[0], "hello")) {
        return 0;
    }
    if ( strcmp(result[1], "welcomeToCs241")) {
        return 0;
    }
    destroy(result);

    // case5: all punch case
    result = camelCaser("@.!");
    // printf("%s", *result);
    if ( strcmp(result[0], "")) return 0;
    if ( strcmp(result[1], "")) return 0;
    if ( strcmp(result[2], "")) return 0;
    destroy(result);

    // case6: all upper case
    result = camelCaser("AJGBOA? SHG NJL !");
    printf("%s\n", *result);
    printf("%s\n", *(result+1));
    // printf("%s\n", *(result+2));
    if ( strcmp(result[0], "ajgboa")) return 0;
    if ( strcmp(result[1], "shgNjl")) return 0;
    // if ( strcmp(result[2], "")) return 0;
    destroy(result);

    // case7: all lower case
    result = camelCaser("jzsfg?akgsfg  sg! gsgg");
    // printf("%s\n", *result);
    // printf("%s\n", *(result+1));
    if ( strcmp(result[0], "jzsfg")) return 0;
    if ( strcmp(result[1], "akgsfgSg")) return 0;
    // if ( strcmp(result[2], "")) return 0;
    destroy(result);

    // // case8: difficult case
    // const char* str18="76\v9 IQ?T: M\bE q78pE\nm Gu\ni?";
    // char* str10_0="769Iq";
    // char* str10_1="t";
    // char* str10_2="eQ78peMGuI";
    // char** output10=camelCaser(str18);
    // printf("%s\n", *output10);
    // printf("%s\n", *(output10+1));
    // printf("%s\n", *(output10+2));
    // if(strcmp(output10[0],str10_0)!=0){return 0;}
    // if(strcmp(output10[1],str10_1)!=0){return 0;}
    // if(strcmp(output10[2],str10_2)!=0){return 0;}
    // destroy(output10);

    // case9: all lower case
    result = camelCaser("jzsfgBISsfg  sg gsgg");
    // printf("%s\n", *result);
    // printf("%s\n", *(result+1));
    if (result[0] != 0) {
        return 0;
    }
    destroy(result);

    // case9: all lower case
    result = camelCaser("   ");
    // printf("%s\n", *result);
    // printf("%s\n", *(result+1));
    if (result[0] != 0) {
        return 0;
    }
    destroy(result);

    return 1;
}
