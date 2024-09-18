/**
 * charming_chatroom
 * CS 341 - Fall 2023
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    int32_t size_buf = htonl(size);
    ssize_t write_bytes = write_all_to_socket(socket, (char*)&size_buf, MESSAGE_SIZE_DIGITS);
    return write_bytes;
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    ssize_t all_read = 0;
    ssize_t remain_byte = count;
    while (remain_byte > 0) {
        ssize_t get_read = read(socket, (void*) (buffer + all_read), remain_byte);
        if (get_read == 0) {
            return 0;
        }
        if (get_read == -1 && errno == EINTR) {
            continue;
        }
        if (get_read > 0) {
            remain_byte -= get_read;
            all_read += get_read;
        }
    }
    return all_read;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    ssize_t all_written = 0;
    ssize_t remain_byte = count;
    while (remain_byte > 0) {
        ssize_t get_write = write(socket, (void*) (buffer + all_written), remain_byte);
        if (get_write == 0) {
            return 0;
        }
        if (get_write == -1 && errno == EINTR) {
            continue;
        }
        if (get_write > 0) {
            remain_byte -= get_write;
            all_written += get_write;
        }
    }
    return all_written;
}
