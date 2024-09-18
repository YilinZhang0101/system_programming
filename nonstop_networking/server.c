/**
 * nonstop_networking
 * CS 341 - Fall 2023
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>

#include "includes/dictionary.h"
#include "includes/vector.h"
#include "format.h"
#include "common.h"

#define MAX_CLIENTS 100
#define FILENAME_SIZE 1024
#define HEADER_SIZE 128
#define HEAD_READ_LIMIT 263

#define min(a, b) (((a) < (b)) ? (a) : (b))

static volatile int serverSocket;
static int epoll_fd;

static char* dir_template;
static dictionary* client_dictionary;
static dictionary* client_dict_filesize;
static vector* filename_vec;
static size_t filename_size;

typedef struct info_client {
	int state;
	verb method;
	char filename[FILENAME_SIZE];
	char header[HEADER_SIZE];
} info_client;

void run_server(char *server_port);
int localfile_write(int socket, FILE* local_file, size_t file_size);
void sig_function();
void print_directory();
void run_epoll();

void parse_request(int client_fd, info_client* info);
void process_header(int client_fd, info_client* info);
int process_method(int client_fd, info_client* info);
void process_error(int client_fd, info_client* info);

void clean_up();
void unlink_direction();
void server_end();
void exit_for_server(int state);


int main(int argc, char **argv) {
    // good luck!
    if (argc != 2) {
        print_server_usage();
        exit(1);
    }
    signal(SIGPIPE, sig_function);

    struct sigaction sigact;
    memset(&sigact, '\0', sizeof(sigact));
    sigact.sa_handler = server_end;
    if (sigaction(SIGINT, &sigact, NULL) == -1) {
        perror("sigact error");
        exit(1);
    }

    filename_vec = string_vector_create();    
    client_dictionary = int_to_shallow_dictionary_create();
    client_dict_filesize = string_to_unsigned_int_dictionary_create();

    print_directory();

    char* server_port = argv[1];
    run_server(server_port);
}

void sig_function() {
    // empty
}

void print_directory() {
    char template[] = "XXXXXX";
    dir_template = mkdtemp(template);
    print_temp_directory(dir_template);
}

void server_end() {
    exit_for_server(1);
}

void exit_for_server(int state) {
    clean_up();
    exit(state);
}

void clean_up() {
    close(epoll_fd);
    
    vector* get_values = dictionary_values(client_dictionary);
    size_t i = 0;
    while (i < vector_size(get_values)) {
        info_client* clean_info = vector_get(get_values, i);
        free(clean_info);
        i ++;
    }
    vector_destroy(get_values);

    unlink_direction();
    
    vector_destroy(filename_vec);
    dictionary_destroy(client_dictionary);
    dictionary_destroy(client_dict_filesize);
}

void unlink_direction() {
    //unlink files in new_direction
    for (size_t i = 0; i < vector_size(filename_vec); i++) {
        char* get_filename = vector_get(filename_vec, i);
        char get_filepath[strlen(dir_template) + strlen(get_filename) + 2];
        memset(get_filepath, 0, strlen(dir_template) + strlen(get_filename) + 2);
        sprintf(get_filepath, "%s/%s", dir_template, get_filename);
        unlink(get_filepath);
    }
    rmdir(dir_template);
}

/**
Taken from Charming_chatroom
 */
void run_server(char *server_port) {
    /*QUESTION 1*/
    /*QUESTION 2*/
    /*QUESTION 3*/
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("server connection error");
        exit(1);
    }

    /*QUESTION 8*/
    int optval = 1;
    int setsockopt_signal = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (setsockopt_signal == -1) {
        perror(NULL);
        exit(1);
    }

    /*QUESTION 4*/
    /*QUESTION 5*/
    /*QUESTION 6*/
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int new_addrinfo = getaddrinfo(NULL, server_port, &hints, &res);
    if (new_addrinfo != 0) {
        const char *get_addrinfo_error = gai_strerror(new_addrinfo);
        fprintf(stderr, "%s\n", get_addrinfo_error);
        if (res != NULL) {
            freeaddrinfo(res);
            res = NULL;
        }
        exit(1);
    }

    /*QUESTION 9*/
    int bind_signal = bind(serverSocket, res->ai_addr, res->ai_addrlen);
    if (bind_signal == -1) {
        perror("bind error");
        if (res != NULL) {
            freeaddrinfo(res);
            res = NULL;
        }
        exit(1);
    }

    /*QUESTION 10*/
    int listen_signal = listen(serverSocket, MAX_CLIENTS);
    if (listen_signal == -1) {
        perror("listen error");
        if (res != NULL) {
            freeaddrinfo(res);
            res = NULL;
        }
    }

    if (res != NULL) {
        freeaddrinfo(res);
        res = NULL;
    }

    run_epoll();
}

void run_epoll() {
    epoll_fd = epoll_create(64);
    if (epoll_fd == -1) {
        perror("epoll error");
        exit(1);
    }

    struct epoll_event event_server;
    memset(&event_server, 0, sizeof(event_server));
    event_server.events = EPOLLIN;
    // associated serverSocket is ready for reading
    event_server.data.fd = serverSocket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event_server);
    // adds the serverSocket to the epoll instance 
    // so that epoll can monitor events on this socket

    struct epoll_event events[MAX_CLIENTS];

    // waits for events on any of the sockets that 
    // have been added to the epoll instance.
    while (1) {
        int epoll_wait_result = epoll_wait(epoll_fd, events, MAX_CLIENTS, 1000);
        // fills the events array with up to MAX_CLIENTS events that occurred on the monitored sockets. 
        // It waits for a maximum of 1000 ms. 
        // If an error occurs during the wait, it exits the program.
        if (epoll_wait_result == -1) {
            exit_for_server(1);
        }

        for (int i = 0; i < epoll_wait_result; i++) {
            int get_event_fd = events[i].data.fd;
            if (get_event_fd != serverSocket) {
                info_client* get_info = dictionary_get(client_dictionary, &get_event_fd);
                if (get_info->state == 0) {
                    // header not parsed
                    process_header(get_event_fd, get_info);
                    continue;
                } 
                else if (get_info->state == 1) {
                    // process method
                    int result_method = process_method(get_event_fd, get_info);
                    if (result_method == -1) {
                        continue;
                    }
                } 
                else {
                    // process error
                    process_error(get_event_fd, get_info);
                }

                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, get_event_fd, NULL);
                free(get_info);
                dictionary_remove(client_dictionary, &get_event_fd);
                shutdown(get_event_fd, SHUT_RDWR);
                close(get_event_fd);
            } 
            else {
                // accept a new incoming connection
                int client_fd = accept(serverSocket, NULL, NULL);
                if (client_fd == -1) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("accept error");
                        exit_for_server(1);
                    }
                    continue;
                }
                struct epoll_event new_event;
                memset(&new_event, 0, sizeof(new_event));
                new_event.events = EPOLLIN;
                new_event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &new_event);
                // set up for the new client_fd to listen for read events (EPOLLIN) 
                // and added to the epoll instance with epoll_ctl.
                
                info_client* get_info = calloc(1, sizeof(info_client));
                dictionary_set(client_dictionary, &client_fd, get_info);
            }
        }
    }
}

void process_header(int client_fd, info_client* info) {
    size_t total_read = 0;
    while ((HEAD_READ_LIMIT - total_read) > 0) {
        ssize_t bytes_read = read(client_fd, info->header + total_read, 1);
        size_t len = strlen(info->header);
        if (info->header[len - 1] == '\n') {
            //read to end
            break;
        }
        if (bytes_read == -1 && errno == EINTR) {
            continue;
        }
        if (bytes_read > 0) {
            total_read += bytes_read;
        }
        else {
            info->state = -1;
            // err_bad_request
            break;
        }
    }
    if (total_read >= HEAD_READ_LIMIT) {
        info->state = -1;
        // err_bad_request
    }
    if (info->state == -1) {
        // err_bad_request
        print_invalid_response();
        struct epoll_event out_event;
        memset(&out_event, 0, sizeof(out_event));
        out_event.events = EPOLLOUT;
        out_event.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &out_event);
        return;
    }

    parse_request(client_fd, info);
}

void parse_request(int client_fd, info_client* info) {
    if (strncmp("PUT", info->header, 3) == 0) {
        strcpy(info->filename, info->header + 4);
        info->filename[strlen(info->filename) - 1] = '\0';
        info->method = PUT;

        size_t file_size;
        char get_filepath[strlen(dir_template) + strlen(info->filename) + 2];
        memset(get_filepath, 0, strlen(dir_template) + strlen(info->filename) + 2);
        sprintf(get_filepath, "%s/%s", dir_template, info->filename);
        int isExisted = 0;
        if (access(get_filepath, F_OK) == 0) {
            isExisted = 1;
        }
        FILE* file = fopen(get_filepath, "w+");
        
        read_all_from_socket(client_fd, (char*)&file_size, sizeof(file_size));
        size_t total_bytes_read = 0;
        size_t blocksize = 512;
        char buf[blocksize];

        size_t max_size = file_size + 5;
        
        while (total_bytes_read < max_size) {
            if (max_size - total_bytes_read < blocksize) {
                blocksize = max_size - total_bytes_read;
            }
            size_t bytes_read = read_all_from_socket(client_fd, buf, blocksize);
            if (bytes_read == 0) {
                break;
            }
            fwrite(buf, sizeof(char), bytes_read, file);
            total_bytes_read += bytes_read;
        }
        fclose(file);

        if (total_bytes_read != file_size) {
            unlink(get_filepath);
            info->state = -2;
            struct epoll_event out_event;
            memset(&out_event, 0, sizeof(out_event));
            out_event.events = EPOLLOUT;
            out_event.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &out_event);
            return;
        }
        if (isExisted == 0) {
            vector_push_back(filename_vec, info->filename);
            filename_size += strlen(info->filename);
        }
        dictionary_set(client_dict_filesize, info->filename, &file_size);
    } 
    else if (strncmp("GET", info->header, 3) == 0) {
        strcpy(info->filename, info->header + 4);
        info->filename[strlen(info->filename) - 1] = '\0';
        info->method = GET;
    } 
    else if (strncmp("DELETE", info->header, 6) == 0) {
        strcpy(info->filename, info->header + 7);
        info->filename[strlen(info->filename) - 1] = '\0';
        info->method = DELETE;
    } 
    else if (strncmp("LIST", info->header, 4) == 0) {
        info->method = LIST;
    } 
    else {
        // invalid
        print_invalid_response();
        info->state = -1;
        struct epoll_event out_event;
        memset(&out_event, 0, sizeof(out_event));
        out_event.events = EPOLLOUT;
        out_event.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &out_event);
        return;
    }

    //header is parsed
    info->state = 1;
    struct epoll_event out_event;
    memset(&out_event, 0, sizeof(out_event));
    out_event.events = EPOLLOUT;
    out_event.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &out_event);
}

int process_method(int client_fd, info_client* info) {
    if (info->method == PUT) {
        // done
        write_all_to_socket(client_fd, "OK\n", 3);
        return 0;
    } 
    else if (info->method == LIST) {
        write_all_to_socket(client_fd, "OK\n", 3);
        size_t get_filesize = vector_size(filename_vec) + filename_size;
        if (vector_size(filename_vec) > 0) {
            get_filesize --;
        }
        write_all_to_socket(client_fd, (char*)&get_filesize, sizeof(get_filesize));
        for (size_t i = 0; i < vector_size(filename_vec); i++) {
            char* get_filename = vector_get(filename_vec, i);
            write_all_to_socket(client_fd, get_filename, strlen(get_filename));
            if ((i + 1) != vector_size(filename_vec)) {
                write_all_to_socket(client_fd, "\n", 1);
            }
        }
        return 0;
    } 
    else if (info->method == GET) {
        char get_filepath[strlen(dir_template) + strlen(info->filename) + 2];
        memset(get_filepath, 0, strlen(dir_template) + strlen(info->filename) + 2);
        sprintf(get_filepath, "%s/%s", dir_template, info->filename);
        FILE* file = fopen(get_filepath, "r");
        if (file == NULL) {
            //file not exist
            info->state = -3;
            struct epoll_event out_event;
            memset(&out_event, 0, sizeof(out_event));
            out_event.events = EPOLLOUT;
            out_event.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &out_event);
            return -1;
        }
        write_all_to_socket(client_fd, "OK\n", 3);
        size_t get_filesize = *(size_t*)dictionary_get(client_dict_filesize, info->filename);
        write_all_to_socket(client_fd, (char*)&get_filesize, sizeof(get_filesize));
        if (localfile_write(client_fd, file, get_filesize) == 0) {
            fclose(file);
            return 0;
        }

        info->state = -3;
        struct epoll_event out_event;
        memset(&out_event, 0, sizeof(out_event));
        out_event.events = EPOLLOUT;
        out_event.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &out_event);
        return -1;
    } 
    else if (info->method == DELETE) {
        char get_filepath[strlen(dir_template) + strlen(info->filename) + 2];
        memset(get_filepath, 0, strlen(dir_template) + strlen(info->filename) + 2);
        sprintf(get_filepath, "%s/%s", dir_template, info->filename);
        if (access(get_filepath, F_OK) != 0) {
            info->state = -3;
            struct epoll_event out_event;
            memset(&out_event, 0, sizeof(out_event));
            out_event.events = EPOLLOUT;
            out_event.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &out_event);
            return -1;
        }
        unlink(get_filepath);
        for (size_t i = 0; i < vector_size(filename_vec); i++) {
            if (strcmp(vector_get(filename_vec, i), info->filename) != 0) {
                continue;
            }
            filename_size -= strlen(info->filename);
            dictionary_remove(client_dict_filesize, info->filename);
            vector_erase(filename_vec, i);
            write_all_to_socket(client_fd, "OK\n", 3);
            return 0;
        }
        
        info->state = -3;
        struct epoll_event out_event;
        memset(&out_event, 0, sizeof(out_event));
        out_event.events = EPOLLOUT;
        out_event.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &out_event);
        return -1;
    }
    return 0;
}

void process_error(int client_fd, info_client* info) {
    write_all_to_socket(client_fd, "ERROR\n", 6);

    if (info->state == -1) {
        write_all_to_socket(client_fd, err_bad_request, strlen(err_bad_request));
    }
    else if (info->state == -2) {
        write_all_to_socket(client_fd, err_bad_file_size, strlen(err_bad_file_size));
    }
    else if (info->state == -3) {
        write_all_to_socket(client_fd, err_no_such_file, strlen(err_no_such_file));
    }
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