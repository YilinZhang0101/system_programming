/**
 * nonstop_networking
 * CS 341 - Fall 2023
 */
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "format.h"

#include "common.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))

char **parse_args(int argc, char **argv);
verb check_args(char **args);

void connect_to_server(const char* host, const char* port);
void exit_for_client(char** args, int exit_code);
void handle_error_response(char* response, ssize_t response_read_num);
void send_request(const char* method, const char* remote, const verb method_verb);
int localfile_write(int socket, FILE* local_file, size_t file_size);

// Global variable
static int sock_fd;
static char** get_args;


/**
Taken from Charming_chatroom
 */
void connect_to_server(const char *host, const char *port) {
    /*QUESTION 1*/
    /*QUESTION 2*/
    /*QUESTION 3*/
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket error");
        exit(1);
    }

    /*QUESTION 4*/
    /*QUESTION 5*/
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    /*QUESTION 6*/
    int new_addrinfo = getaddrinfo(host, port, &hints, &res);
    if (new_addrinfo != 0) {
        const char *get_addrinfo_error = gai_strerror(new_addrinfo);
        fprintf(stderr, "%s\n", get_addrinfo_error);
        if (res != NULL) {
            freeaddrinfo(res);
            res = NULL;
        }
        exit(1);
    }

    /*QUESTION 7*/
    int connect_signal = connect(sock_fd, res->ai_addr, res->ai_addrlen);
    // The file descriptor of the socket you want to use to establish the connection.
    if (connect_signal == -1) {
        if (res != NULL){
            freeaddrinfo(res);
            res = NULL;
        }
        perror("connect error");
        exit(1);
    }

    if (res != NULL) {
        freeaddrinfo(res);
        res = NULL;
    }
}

void exit_for_client(char** args, int status) {
    close(sock_fd);
    free(args);
    exit(status);
}

void send_request(const char* method, const char* remote, const verb method_verb) {    
    if (method_verb == LIST || method_verb == GET || method_verb == DELETE || method_verb == PUT) {
        char* buff;
        size_t size_buff;
        if (method_verb == LIST) {
            size_buff = strlen(method) + 2;
            buff = calloc(1, size_buff);
            sprintf(buff, "%s\n", method);
        } else {
            size_buff = strlen(method) + strlen(remote) + 3;
            buff = calloc(1, size_buff);
            sprintf(buff, "%s %s\n", method, remote);
        }

        size_t buff_len = strlen(buff);
        ssize_t write_num = write_all_to_socket(sock_fd, buff, buff_len);
        free(buff);
        if (write_num == -1 || write_num < (ssize_t)buff_len) {
            print_connection_closed();
            exit_for_client(get_args, 1);
        }
    }
}

void handle_get(const char* local) {
    // Additional GET logic here...
    size_t file_size;
    read_all_from_socket(sock_fd, (char*)&file_size, sizeof(file_size));
    FILE* get_local = fopen(local, "w+");
    if (get_local == NULL) {
        print_connection_closed();
        exit_for_client(get_args, 1);
    }
    size_t total_read_num = 0;
    size_t block_size = 512;
    char buf[block_size];
    //in order to detect too much data error, enlarge the max size
    size_t max_size = file_size + 5;
    //read data from server and write to local file
    while (max_size > 0) {
        block_size = min(block_size, max_size);
        size_t read_num = read_all_from_socket(sock_fd, buf, block_size);
        if (read_num == 0) {
            break;
        }
        fwrite(buf, sizeof(char), read_num, get_local);
        max_size -= read_num;
        total_read_num += read_num;
    }

    if (total_read_num > file_size) {
        print_received_too_much_data();
        exit_for_client(get_args, 1);
    } 
    else if (total_read_num < file_size) {
        print_too_little_data();
        exit_for_client(get_args, 1);
    }
    fclose(get_local);
}

void handle_list() {
    // Additional LIST logic here...
    size_t size;
    read_all_from_socket(sock_fd, (char*)&size, sizeof(size));
    //in order to detect too much data error, enlarge the max size
    size_t max_size = size + 5;
    char buf[max_size];
    memset(buf, 0, max_size);
    ssize_t read_num = read_all_from_socket(sock_fd, buf, max_size - 1);
    //check error
    if (read_num == -1) {
        print_connection_closed();
        exit_for_client(get_args, 1);
    }

    if (read_num < (ssize_t)size) {
        print_too_little_data();
        exit_for_client(get_args, 1);
    } 
    else if (read_num > (ssize_t)size) {
        print_received_too_much_data();
        exit_for_client(get_args, 1);
    }
    printf("%zu%s", size, buf);
}


void handle_response(verb method_verb, const char* local) {
    char* response = calloc(1, 4);
    ssize_t response_read_num = read_all_from_socket(sock_fd, response, 3);
    if (response_read_num == -1) {
        print_connection_closed();
        exit_for_client(get_args, 1);
    }
    
    if (strcmp(response, "OK\n") == 0) {
        printf("OK\n");
        if (method_verb == PUT || method_verb == DELETE) {
            print_success();
        }
        else if (method_verb == GET) {
            handle_get(local);
        }
        else if (method_verb == LIST){
            handle_list();
        }
    } else {
        handle_error_response(response, response_read_num);
    }
    free(response);
}

void handle_error_response(char* response, ssize_t response_read_num) {
    // Error response logic here...
    char* error = "ERROR\n";
    response = realloc(response, strlen(error) + 1);
    read_all_from_socket(sock_fd, response + response_read_num, strlen(error) - response_read_num);
    response[strlen(error)] = 0;
    if (strcmp(response, error) != 0) {
        print_invalid_response();
        exit_for_client(get_args, 1);
    }
    char error_mes[21];
    ssize_t read_error_success = read_all_from_socket(sock_fd, error_mes, 20);
    error_mes[20] = 0;
    
    if (read_error_success == -1) {
        print_connection_closed();
        exit_for_client(get_args, 1);
    }
    print_error_message(error_mes);
}

int localfile_write(int socket, FILE* local_file, size_t file_size) {
    size_t init_file_size = file_size;
    size_t block_size = 512;
    char buf[block_size];
    //write block_size of bytes each time
    while (file_size > 0) {
        block_size = min(file_size, block_size);
        size_t read_num = fread(buf, sizeof(char), block_size, local_file);
        if (read_num == 0) {
            break;
        }
        if (write_all_to_socket(socket, buf, read_num) == -1) {
            return -1;
        }
        file_size -= read_num;
    }
    file_size = init_file_size;
    return 0;
}

int main(int argc, char** argv) {
    get_args = parse_args(argc, argv);
    verb method_verb = check_args(get_args);
    char* server_ip = get_args[0];
    char* server_port = get_args[1];
    char* get_verb = get_args[2];
    char* remote = get_args[3];
    char* local = get_args[4];
    
    connect_to_server(server_ip, server_port);
    
    // Send client request based on the method verb
    send_request(get_verb, remote, method_verb);

    if (method_verb == PUT) {
        //write local file to server
        struct stat get_status;
        if (stat(local, &get_status) == -1) {
            exit(1);
        }
        size_t file_size = get_status.st_size;
        write_all_to_socket(sock_fd, (const char*)&file_size, sizeof(file_size));
        //open local file
        FILE* local_file = fopen(local, "r");
        if (local_file == NULL) {
            //file not exist
            print_connection_closed();
            exit_for_client(get_args, 1);
        }
        int write_local_success = localfile_write(sock_fd, local_file, file_size);
        if (write_local_success == -1) {
            //close local file
            fclose(local_file);
            print_connection_closed();
            exit_for_client(get_args, 1);
        }
        //close local file
        fclose(local_file);
    }

    // Shutdown the writing end after sending the request
    if (shutdown(sock_fd, SHUT_WR) == -1) {
        perror("shutdown after sending request");
    }

    // Handle server response
    handle_response(method_verb, local);

    // Shutdown the reading end and exit
    if (shutdown(sock_fd, SHUT_RD) == -1) {
        perror("shutdown the reading");
    }
    exit_for_client(get_args, 0);
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
