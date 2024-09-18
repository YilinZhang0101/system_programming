/**
 * deepfried_dd
 * CS 341 - Fall 2023

 * collaborate with yifan20
 */
#include "format.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

static size_t full_in = 0;
static size_t partial_in = 0;
static size_t full_out = 0;
static size_t partial_blocks_out = 0;
static size_t total_copied = 0;

static struct timespec start_time, end_time;
void handle_sigusr1(int sig);


int main(int argc, char *argv[]) {
    int opt;
    char *in_filename = NULL;
    char *out_filename = NULL;
    size_t block_size = 512;
    size_t block_copied = 0;
    size_t skip_input_blocks = 0;
    size_t skip_output_blocks = 0;

    FILE *file_in = stdin;
    FILE *file_out = stdout;

    if (argc > 1) {
        while ((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
            switch (opt) {
                case 'i':
                    if (optarg != 0) {
                        in_filename = malloc(strlen(optarg) + 1);
                        strcpy(in_filename, optarg);
                    }
                    break;
                case 'o':
                    if (optarg != 0) {
                        out_filename = malloc(strlen(optarg) + 1);
                        strcpy(out_filename, optarg);
                    }
                    break;
                case 'b':
                    if (optarg != 0) {
                        block_size = atoi(optarg);
                    }
                    break;
                case 'c':
                    if (optarg != 0) {
                        block_copied = atoi(optarg);
                    }
                    break;
                case 'p':
                    if (optarg != 0) {
                        skip_input_blocks = atoi(optarg);
                    }
                    break;
                case 'k':
                    if (optarg != 0) {
                        skip_output_blocks = atoi(optarg);
                    }
                    break;
                default:
                    exit(1);
            }
        }
    }

    if (in_filename != NULL) {
        file_in = fopen(in_filename, "rb");
        if (file_in == NULL) {
            print_invalid_input(in_filename);
            free(in_filename);
            exit(1);
        }
    }

    if (out_filename != NULL) {
        file_out = fopen(out_filename, "wb");
        if (file_out == NULL) {
            print_invalid_output(out_filename);
            free(out_filename);
            exit(1);
        }
    }

    if (file_in != stdin) {
        // inputfile is valid
        fseek(file_in, skip_input_blocks * block_size, SEEK_SET);
    }
    if (file_out != stdout) {
        // outputfile is valid
        fseek(file_out, skip_output_blocks * block_size, SEEK_SET);
    }
    
    char *buffer = malloc(block_size);
    if (buffer == NULL) {
        perror("malloc");
        exit(1);
    }

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    signal(SIGUSR1, handle_sigusr1);

    size_t bytes_read, bytes_written;
    while ((bytes_read = fread(buffer, 1, block_size, file_in)) > 0) {
        if (bytes_read == block_size) {
            full_in++;
        } else {
            partial_in++;
        }

        bytes_written = fwrite(buffer, sizeof(char), bytes_read, file_out);
        fflush(file_out);
        if (bytes_written == block_size) {
            full_out++;
        } else {
            partial_blocks_out++;
        }

        total_copied += bytes_written;

        if (block_copied && full_in >= block_copied) {
            break;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double diff_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    print_status_report(full_in, partial_in, full_out, partial_blocks_out, total_copied, diff_time);
    fflush(stdout);

    free(buffer);
    if (in_filename != NULL) {
        free(in_filename);
    }
    if (out_filename != NULL) {
        free(out_filename);
    }
    if (file_in != stdin) {
        fclose(file_in);
    }
    if (file_out != stdout) {
        fclose(file_out);
    }

    return 0;
}

void handle_sigusr1(int sig) {
    if (sig == SIGUSR1) {
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        double duration = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
        print_status_report(full_in, partial_in, full_out, partial_blocks_out, total_copied, duration);
        fflush(stdout);
    }
}