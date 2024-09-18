/**
 * utilities_unleashed
 * CS 341 - Fall 2023
 */
#include "format.h"
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {
    if(argc < 2) {
        print_time_usage();
        // return 0;
    }

    struct timespec time_start, time_end;
    int status;
    clock_gettime(CLOCK_MONOTONIC, &time_start);

    pid_t new_child = fork();
    if (new_child == -1) {
        print_fork_failed();
        // return 0;
    } 
    
    if (new_child > 0) {        // child
        pid_t pid = waitpid(new_child, &status, 0);
        if (pid == -1) {
            exit(1);
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            clock_gettime(CLOCK_MONOTONIC, &time_end);
            double dur_nsec = (time_end.tv_nsec - time_start.tv_nsec)/1e9;
            double dur_sec = time_end.tv_sec - time_start.tv_sec;
            display_results(argv, dur_nsec + dur_sec);
        }
    } else {                    // parent
        execvp(argv[1], argv + 1);
        print_exec_failed();
    }
    return 0;
}
