/**
 * mapreduce
 * CS 341 - Fall 2023
 */
#include "utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    if (argc != 6) {
        print_usage();
        exit(1);
    }
    char* input_file = argv[1];
    char* output_file = argv[2];
    char* mapper_executable = argv[3];
    char* reducer_executable = argv[4];
    int mapper_count = atoi(argv[5]);

    // Create an input pipe for each mapper.
    int map_pipes[mapper_count * 2];
    // each pipe consists of two file descriptors: 
    // one for reading and one for writing.
    for (int i = 0; i < mapper_count; i ++) {
        pipe(map_pipes + 2 * i);
        // Read end of pipe for each mapper
    }

    // Create one input pipe for the reducer.
    int reducer_pipes[2];
    pipe(reducer_pipes);
    // Read end of pipe for reducer

    // Open the output file.
    int output_fp = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
    if (output_fp == -1) {
        exit(1);
    }

    // Start a splitter process for each mapper.
    int split_pid[mapper_count];
    for (int i = 0; i < mapper_count; i++) {
        split_pid[i] = fork();
        if (split_pid[i] == -1) {
            exit(1);
        }
        else {
            if (split_pid[i] == 0) {  
            // it's for child
                // itoa(i, str_i, 10);
                close(map_pipes[2 * i]);
                // close read end, only need to write end
                if (dup2(map_pipes[2 * i + 1], 1) == -1) {
                    // 1 in this case, which stands for standard output or stdout) refer to the same open file as the first argument.
                    // redirecting the standard output (stdout) of the child process to the write end of the current mapper's pipe.
                    exit(1);
                }
                char str_i[10];
                sprintf(str_i, "%d", i);
                execl("splitter", "splitter", input_file, argv[5], str_i, NULL);
                // replace the current process image with a new process image
                // first: the path to the executable program that should be run
                // second: name of the program itself.
                // third: name of input file
                // fifth: The string representation of the current splitter's/mapper's index
                exit(1);
                // if this command fail to execute
            }
        }
    }

    // Start all the mapper processes.
    int map_pids[mapper_count];
    for (int i = 0; i < mapper_count; i++) {
        close(map_pipes[2 * i + 1]);
        // close write end
        map_pids[i] = fork();
        if (map_pids[i] == -1) {
            exit(1);
        }
        else {
            if (map_pids[i] == 0) {
                close(reducer_pipes[0]);
                // close read end
                if ((dup2(map_pipes[i * 2], 0) == -1) || (dup2(reducer_pipes[1], 1) == -1)) {
                    //The value 0 represents the standard input
                    exit(1);
                }
                execl(mapper_executable, mapper_executable, NULL);
                exit(1);
            }
        }
    }

    // Start the reducer process.
    int reducer_pid = fork();
    if (reducer_pid == -1) {
        exit(1);
    }
    else {
        if (reducer_pid == 0) {
            close(reducer_pipes[1]);
            // couldn't write into
            if (dup2(reducer_pipes[0], 0) == -1) {
                exit(1);
            }
            if (dup2(output_fp, 1) == -1) {
                exit(1);
            }
            // check if can read from reducer read end, write into output
            execl(reducer_executable, reducer_executable, NULL);
            // start executing the code from mapper_executable
            exit(1);
        }
    }

    for (int i = 0; i < mapper_count * 2; i++) {
        close(map_pipes[i]);
        if (i < 2) {
            close(reducer_pipes[i]);
            // prevent deadlocks and to allow EOF to be detected 
        }
    }

    // Wait for the reducer to finish.
    int status;
    waitpid(reducer_pid, &status, 0);
    if (WIFEXITED(status)) {
        print_nonzero_exit_status(reducer_executable, WEXITSTATUS(status));
    }
    
    // Print nonzero subprocess exit codes.
    int split_status;
    int map_status;
    for (int i = 0; i < mapper_count; i++) {
        waitpid(split_pid[i], &split_status, 0);
        waitpid(map_pids[i], &map_status, 0);
        if (WIFEXITED(split_status)) {
            print_nonzero_exit_status("splitter", WEXITSTATUS(status));
        }
        if (WIFEXITED(map_status)) {
            print_nonzero_exit_status(mapper_executable, WEXITSTATUS(status));
        }
    }

    // Count the number of lines in the output file.
    print_num_lines(output_file);
    close(output_fp);
    return 0;
}
