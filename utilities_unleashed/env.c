/**
 * utilities_unleashed
 * CS 341 - Fall 2023
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "format.h"
#include <sys/types.h>
#include <sys/wait.h>


void before_dash(char* input) {
    size_t len = strlen(input);
    char *key = malloc((len + 1) * sizeof(char));
    char *value = malloc((len + 1) * sizeof(char));

    if (strstr(input, "=") == NULL) {
        free(key);
        free(value);   
        print_env_usage();
    }

    key = strtok(input, "=");
    if (key != NULL) {
        value = strtok(NULL, "=");
    }

    if (value[0] == '%') {
        value = getenv(value + 1);
    }

    if (value == NULL || setenv(key, value, 1) == -1) {
        free(key);
        free(value);   
        print_environment_change_failed();
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_env_usage();
    }

    int status = 0;
    pid_t new_child = fork();
    if (new_child == -1) {
        print_fork_failed();
    }
    if (new_child > 0) {         // child
        pid_t pid = waitpid(new_child, &status, 0);
        if (pid == -1) {
            exit(1);
        }
    } else {                     // parent
        // int index_dash = -1;
        // for (int i = 1; i < argc; i++) {
        //     if (!strcmp(argv[i], "--")) {
        //         index_dash = i;
        //         break;
        //     }
        //     before_dash(argv[index_dash]);
        // }

        // if (index_dash == -1) {          // Cannot find -- in arguments
        //     print_env_usage();
        // }
        // if (index_dash == (argc - 1)) {    //Cannot find cmd after --
        //     print_env_usage();
        // }

        int after = 0;
        int before = 0;
        int exec_args = 0;
        for (int i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "--")) {
                exec_args = 1;
            } else if (exec_args) {
                before++;
            } else {
                after++;
            }
        }

    if ((after == 0 && before == 0) || exec_args == 0) {
        print_env_usage();
        return 0;
    }
        
        execvp(argv[before + 2], argv + before + 2);
        print_exec_failed();
    }
    return 0;
}


// int main(int argc, char *argv[]) {
//     if (argc < 3) {
//         print_env_usage();
//     }

//     int after = 0;
//     int before = 0;
//     int exec_args = 0;
//     for (int i = 1; i < argc; i++) {
//         if (!strcmp(argv[i], "--")) {
//             exec_args = 1;
//         } else if (exec_args) {
//             before++;
//         } else {
//             after++;
//         }
//     }

//     if ((after == 0 && before == 0) || exec_args == 0) {
//         print_env_usage();
//         return 0;
//     }

//     char** env_variables = (char**) malloc(after * sizeof(char*));
//     char** exec_inputs = (char**) malloc((before + 1) * sizeof(char*));

//     memcpy(env_variables, &argv[1], after * sizeof(char*));
//     memcpy(exec_inputs, &argv[after + 2], before * sizeof(char*));

//     exec_inputs[before] = NULL;

//     int status;
//     pid_t child = fork();
//     if (child < 0) {
//         free(exec_inputs);
//         free(env_variables);
//         print_fork_failed();
//     }
    
//     if (child > 0) {
//         pid_t pid = waitpid(child, &status, 0);
//         if (pid != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {

//         }
//     } else {
//         for (int i = 0; i < after; i++) {
//             char* curr_pair = env_variables[i];
//             if (strstr(curr_pair, "=") == NULL) {
//                 free(exec_inputs);
//                 free(env_variables);   
//                 print_env_usage();
//             }
//             char* curr_key = strtok(curr_pair, "=");
//             char* curr_val = strtok(NULL, "=");

//             if (curr_val[0] == '%') {
//                 curr_val = getenv(curr_val+1);
//                 if (curr_val == NULL) {
//                     free(exec_inputs);
//                     free(env_variables);   
//                     print_environment_change_failed();
//                 }
//             }

//            if(setenv(curr_key, curr_val, 1) == -1) {
//                 free(exec_inputs);
//                 free(env_variables);   
//                 print_environment_change_failed();
//            }

//         }

//         execvp(exec_inputs[0], exec_inputs);
//         print_exec_failed();
//     }
    
//     free(exec_inputs);
//     free(env_variables);
//     return 0;
// }