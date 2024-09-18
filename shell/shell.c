/**
 * shell
 * CS 341 - Fall 2023
 */

#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef struct process {
    char *command;
    pid_t pid;
} process;
static process* new_fore_p = NULL; 
//store the currently running foreground process

void check_cwd();
void sig_handler(int);

void shell_h(char* arg);
void shell_f(char* arg);
int command_exec(char* command_line, char*** argv);
int symbolic_dicide(char* command_line);
void read_args(char* command_line, int* argc, char*** argv);
int exec_buildin(char* command, int argc, char** argv);

int handle_cd_command(int argc, char** argv);
int handle_history_command();
int handle_call_num_command(char** argv);
int handle_latest_similar_command(char** argv);
int handle_ps_command();
void make_doing_check(char** argv, char* command, int pid);

int check_background(int argc, char** argv);
int make_fork(char* command, int argc, char** argv);

void build_fore_p(pid_t pid, const char* command);
void free_foreground_process();
int run_child(const char* command, char** argv, int isBackground);
int run_parent(pid_t child, int isBackground);

void write_information(int pid, char* command_line);
void rewrite_information();
void destroy_argv(char** argv);
void processes_kill();
void shell_destroy();


#define PATHMAXSIZE 256

static char* cur_dir = NULL;   //store current working directory
static vector* history_command = NULL;
static char* file_name = NULL;
static vector* vec_information = NULL;

static vector* command_words = NULL; //store the string of args split by ' ' from sstr
static char* first_command = NULL;  //store the string of first command if there is operator
static char* second_command = NULL; //store the string of second command if there is operator
static char** old_argv = NULL; //store the array of arguments of a command
static int add_history = 0;  //mark whether to store the command to history_command
static FILE* command_output = 0; //file that need to store the output of a command

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    signal(SIGINT, sig_handler);
    // void (*signal(int sig, void (*func)(int)))(int)
    cur_dir = malloc(sizeof(char) * PATHMAXSIZE);
    history_command = string_vector_create();
    vec_information = shallow_vector_create();
    write_information(getpid(), *argv);
    int new_option;
    if (argc >= 2) {
        while ((new_option = getopt(argc, argv, "h:f:")) != -1) {
        // int getopt(int argc, char* const argv[], const char* optstring)
        // optstring: "h:", h option with args, "ab" a and b option
            if (new_option == 'h') {
                if (!optarg) {
                    print_usage();
                    processes_kill();
                    shell_destroy();
                    exit(1);
                }
                shell_h(optarg);
            }
            else if (new_option == 'f') {
                if (!optarg) {
                    print_usage();
                    processes_kill();
                    shell_destroy();
                    exit(1);
                }
                shell_f(optarg);
            }
            else {         // new_option == '?'
                print_usage();
                processes_kill();
                shell_destroy();
                exit(1);
            }
        }
    }
    check_cwd();
    print_prompt(cur_dir, getpid());
    // write at first
    char* buffer = NULL;
    size_t capacity = 0;
	while (getline(&buffer, &capacity, stdin) != -1) {
        // user type the command
        waitpid(0, NULL, WNOHANG);
        // wait for process with group id equals current process
        if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
            // after return, then read a command
        }
        command_exec(buffer, &old_argv);
        check_cwd();
        print_prompt(cur_dir, getpid());
        // write at the end
	}
    processes_kill();
    shell_destroy();
    exit(0);

    return 0;
}

void sig_handler(int signal) {
    //TODO
    if (signal == SIGINT) {
        if (new_fore_p) {
            pid_t cur_pid = new_fore_p->pid;
            char* cur_command = new_fore_p->command;
            if (kill(cur_pid, SIGINT) == -1) {
                print_no_process_found(cur_pid);
            }
            else {
                print_killed_process(cur_pid, cur_command);
            }
            free(new_fore_p->command);
            free(new_fore_p);
            new_fore_p = NULL;
        }
    }
}


void shell_h(char* input) {
    FILE* store_history = fopen(input, "a+");
    // FILE *fopen(char *new_file, char *mode)
    // a+: read + update
    file_name = get_full_path(input);
    //store path to file_name
    if (store_history == NULL) {
        print_history_file_error();
        processes_kill();
        shell_destroy();
        exit(1);
    }
    char* buffer = NULL;
	size_t capacity = 0;
    // read execution
	while (getline(&buffer, &capacity, store_history) != -1) {
        if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        vector_push_back(history_command, buffer);
        // h: store the history into file,  but first for !history use
        // no need to execute
	}
    buffer = NULL;
    fclose(store_history);
}

//handle -f
void shell_f(char* input) {
    FILE* do_file = fopen(input, "r");
    if (do_file == NULL) {
        print_script_file_error();
        processes_kill();
        shell_destroy();
        exit(1);
    }
    char* buffer = NULL;
    size_t capacity = 0;

    check_cwd();
    print_prompt(cur_dir, getpid());
	while (getline(&buffer, &capacity, do_file) != -1) {
        waitpid(0, NULL, WNOHANG);
        //get the input
        if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        command_exec(buffer, &old_argv);
        check_cwd();
        print_prompt(cur_dir, getpid());
	}
    fclose(do_file);
    do_file = NULL;
    
    processes_kill();
    shell_destroy();
    exit(0);
}

void check_cwd() {
    if (!getcwd(cur_dir, PATHMAXSIZE)) {
        // #include <dir.h> 
        // char *getcwd(char *buf, int n);
        // current dictionary absolute path to buffer, n - size
        processes_kill();
        shell_destroy();
        exit(1);
    }
}

int command_exec(char* command_line, char*** argv) {
    int flag = symbolic_dicide(command_line);
    // decide which symbolic to choose
    int argc = 0;
    int ret_buildin = 0;    
    // get return for if exec_buildin failed
    int ret_fork = 0;    
    // get return for if fork failed
    int ret_flag = 0;
    // 1: ret_buildin
    // 2: ret_fork
    int ret_noeffect = 0;

    int get_stdout = 0;   
    //saved file descriptor for stdout
    int get_stdint = 0;
    
    if (!flag) {
        read_args(command_line, &argc, argv);
        ret_buildin = exec_buildin(command_line, (int)argc, *argv);
        ret_flag = 1;
        if (ret_buildin == 2) {
            ret_fork = make_fork(command_line, (int)argc, *argv);
            // not build-in command, need fork
            ret_flag = 2;
        }
    }
    else {
        if (flag == 1) {            // &&
            read_args(first_command, &argc, argv);
            ret_buildin = exec_buildin(first_command, (int)argc, *argv);
            ret_flag = 1;
            if (ret_buildin == 2) {
                //not built-in command, try exec external command
                ret_fork = make_fork(first_command, (int)argc, *argv);
                ret_flag = 2;
            }
            destroy_argv(*argv);
            *argv = NULL;
            //if first command success, exec second command
            if (ret_fork == 0) {
                read_args(second_command, &argc, argv);
                ret_noeffect = exec_buildin(second_command, (int)argc, *argv);
                if (ret_noeffect == 2) {
                    make_fork(second_command, (int)argc, *argv);
                }
            }
        }
        else if (flag == 2) {        // ||
            read_args(first_command, &argc, argv);
            ret_buildin = exec_buildin(first_command, (int)argc, *argv);
            ret_flag = 1;
            if (ret_buildin == 2) {
                //not built-in command, try exec external command
                ret_fork = make_fork(first_command, (int)argc, *argv);
                ret_flag = 2;
            }
            destroy_argv(*argv);
            *argv = NULL;
            //if first command fail, exec second command
            if (ret_fork == 1) {
                read_args(second_command, &argc, argv);
                ret_noeffect = exec_buildin(second_command, (int)argc, *argv);
                if (ret_noeffect == 2) {
                    make_fork(second_command, (int)argc, *argv);
                }
            }
        }
        else if (flag == 3) {        // ;
            read_args(first_command, &argc, argv);
            ret_noeffect = exec_buildin(first_command, (int)argc, *argv);
            if (ret_noeffect == 2) {
                make_fork(first_command, (int)argc, *argv);
            }
            destroy_argv(*argv);
            *argv = NULL;
            //exec second command
            read_args(second_command, &argc, argv);
            ret_noeffect = exec_buildin(second_command, (int)argc, *argv);
            if (ret_noeffect == 2) {
                make_fork(second_command, (int)argc, *argv);
            }
        }
        else {
            get_stdout = dup(fileno(stdout));
            get_stdint = dup(fileno(stdin));
            //get the file name in (*argv)[0]
            read_args(second_command, &argc, argv);
            //set file as command_output
            switch (flag) {
                case 4:
                    command_output = fopen((*argv)[0], "w+");
                    break;
                case 5:
                    command_output = fopen((*argv)[0], "a+");
                    break;
                case 6:
                    command_output = fopen((*argv)[0], "r");
                    break;
            }
            
            if (command_output == NULL) {
                print_redirection_file_error();
                return 1;
            }
            destroy_argv(*argv);
            *argv = NULL;

            if (flag == 6) {
                dup2(fileno(command_output), fileno(stdin));
                if (command_output) { 
                    fclose(command_output); 
                    command_output = NULL; 
                }
                //exec the command
                read_args(first_command, &argc, argv);
                ret_noeffect = exec_buildin(first_command, (int)argc, *argv);
                if (ret_noeffect == 2) {
                    make_fork(first_command, (int)argc, *argv);
                }
                dup2(get_stdint, fileno(stdin));
                close(get_stdint);
                add_history = 1;
            }
            else {
                read_args(first_command, &argc, argv);
                ret_noeffect = exec_buildin(first_command, (int)argc, *argv);
                if (ret_noeffect == 2) {
                    make_fork(first_command, (int)argc, *argv);
                }
                fflush(stdout);
                if (command_output) { 
                    fclose(command_output); 
                    command_output = NULL; 
                }
                dup2(get_stdout, fileno(stdout));
                close(get_stdout);
                add_history = 1;
            }
        }
        // else if (flag == 4) {
        //     get_stdout = dup(fileno(stdout));
        //     //get the file name in (*argv)[0]
        //     read_args(second_command, &argc, argv);
        //     //set file as command_output
        //     command_output = fopen((*argv)[0], "w+");
        //     if (command_output == NULL) {
        //         print_redirection_file_error();
        //         return 1;
        //     }
        //     destroy_argv(*argv);
        //     *argv = NULL;
        //     //exec the command
        //     read_args(first_command, &argc, argv);
        //     ret_noeffect = exec_buildin(first_command, (int)argc, *argv);
        //     if (ret_noeffect == 2) {
        //         make_fork(first_command, (int)argc, *argv);
        //     }
        //     fflush(stdout);
        //     if (command_output) { 
        //         fclose(command_output); 
        //         command_output = NULL; 
        //     }
        //     dup2(get_stdout, fileno(stdout));
        //     close(get_stdout);
        //     add_history = 1;
        // }
        // else if (flag == 5) {
        //     get_stdout = dup(fileno(stdout));
        //     //get the file name in (*argv)[0]
        //     read_args(second_command, &argc, argv);
        //     //open the file
        //     command_output = fopen((*argv)[0], "a+");
        //     if (command_output == NULL) {
        //         print_redirection_file_error();
        //         return 1;
        //     }
        //     destroy_argv(*argv);
        //     *argv = NULL;
        //     //exec the command
        //     read_args(first_command, &argc, argv);
        //     ret_noeffect = exec_buildin(first_command, (int)argc, *argv);
        //     if (ret_noeffect == 2) {
        //         make_fork(first_command, (int)argc, *argv);
        //     }
        //     fflush(stdout);
        //     if (command_output) { 
        //         fclose(command_output); 
        //         command_output = NULL; 
        //     }
        //     dup2(get_stdout, fileno(stdout));
        //     close(get_stdout);
        //     add_history = 1;
        // }
        // else {
        //     get_stdint = dup(fileno(stdin));
        //     //get the file name in (*argv)[0]
        //     read_args(second_command, &argc, argv);
        //     //open the file
	    //     command_output = fopen((*argv)[0], "r");
        //     if (command_output == NULL) {
        //         print_redirection_file_error();
        //         return 1;
        //     }
        //     destroy_argv(*argv);
        //     *argv = NULL;
        //     dup2(fileno(command_output), fileno(stdin));
        //     if (command_output) { 
        //         fclose(command_output); 
        //         command_output = NULL; 
        //     }
        //     //exec the command
        //     read_args(first_command, &argc, argv);
        //     ret_noeffect = exec_buildin(first_command, (int)argc, *argv);
        //     if (ret_noeffect == 2) {
        //         make_fork(first_command, (int)argc, *argv);
        //     }
        //     dup2(get_stdint, fileno(stdin));
        //     close(get_stdint);
        //     add_history = 1;
        // }
    }

    //store the command to history_command if needed
    if (add_history) {
        vector_push_back(history_command, command_line);
    }
    if (first_command) { 
        free(first_command); 
        first_command = NULL; 
    }
    if (second_command) { 
        free(second_command); 
        second_command = NULL; 
    }
    destroy_argv(*argv);
    *argv = NULL;
    fflush(stdout);

    if (ret_flag == 1) {
        return ret_buildin;
    }
    return ret_fork;
}

// decide operators
int symbolic_dicide(char* command_line) {
    size_t len = strlen(command_line);
    char* str = NULL;
    int flag = 0;
    
    if (strstr(command_line, "&&")) {
        str = strstr(command_line, "&&");
        flag = 1;
    }
    else if (strstr(command_line, "||")) {
        str = strstr(command_line, "||");
        flag = 2;
    }
    else if (strstr(command_line, ";")) {
        str = strstr(command_line, ";");
        flag = 3;
    }
    else if (strstr(command_line, ">") && (!strstr(command_line, ">>"))) {
        str = strstr(command_line, ">");
        flag = 4;
    }
    else if (strstr(command_line, ">>")) {
        str = strstr(command_line, ">>");
        flag = 5;
    }
    else if (strstr(command_line, "<")) {
        str = strstr(command_line, "<");
        flag = 6;
    }

    // printf("%d\n", flag);
    // printf("%zu\n", (size_t)str - (size_t)command_line);

    if (str != NULL) {
        size_t len_first = (size_t)str - (size_t)command_line;
        size_t len_second = len - len_first - 2;
        first_command = malloc(sizeof(char) * len_first);
        second_command = malloc(sizeof(char) * len_second);
        // printf("%zu\n", len_first);
        // printf("%zu\n", len_second);

        strncpy(first_command, command_line, (len_first + 1));
        first_command[len_first] = '\0';
        strncpy(second_command, str + 2, (len_second + 1));
        second_command[len_second] = '\0';

        // printf("%s\n", first_command);
        // printf("%s\n", second_command);
    }
    return flag;
}

void read_args(char* command_line, int* argc, char*** argv) {
    char* token;
    int count = 0;
    const char deli[2] = " ";
    // choose the delimiter
    char** arguments = (char**)calloc(strlen(command_line), sizeof(char*));

    // start tokenization of the command_line
    token = strtok(command_line, deli);

    // walk through other tokens
    while (token != NULL) {
        arguments[count] = strdup(token);
        size_t len = strlen(arguments[count]);
        if (arguments[count][len - 1] == '\n') {
            arguments[count][len - 1] = '\0';
        }
        count++;
        token = strtok(NULL, deli);
        // move to the next token
    }
    arguments[count] = NULL;

    *argc = count;
    *argv = arguments;
}

int handle_cd_command(int argc, char** argv) {
    int flag = 0;
    // 1: fail to cd
    if (argc < 2) {
        print_no_directory("");
        flag = 1;
        return flag;
    }
    if (chdir(argv[1]) == -1) {
        print_no_directory(argv[1]);
        flag = 1;
        return flag;
    }
    return flag;
}

int handle_history_command() {
    size_t command_size = vector_size(history_command);
    for (size_t i = 0; i < command_size; i++) {
        print_history_line(i, vector_get(history_command, i));
    }
    return 0;
}

int handle_call_num_command(char** argv) {
    if (!(*(argv[0] + 1))) {
        print_invalid_index();
        return 1;
    }

    size_t idx = atoi(argv[0] + 1);
    size_t len = vector_size(history_command);
    if (idx >= len) {
        print_invalid_index();
        add_history = 0;
        return 1;
    }
    print_command(vector_get(history_command, idx));
    char **new_argv;
    int ret = command_exec(vector_get(history_command, idx), &new_argv);
    destroy_argv(new_argv);
    new_argv = NULL;
    add_history = 0;
    return ret;
}

int handle_latest_similar_command(char** argv) {
    int idx = vector_size(history_command) - 1;
    if (!strcmp(argv[0], "!")) {
        char* last_one = vector_get(history_command, idx);
        print_command(last_one);
        char **new_argv;
        int res = command_exec(last_one, &new_argv);
        destroy_argv(new_argv);
        new_argv = NULL;
        add_history = 0;
        return res;
    }

    // int idx = vector_size(history_command) - 1;
    for (; idx >= 0; idx--) {
        char* cur = vector_get(history_command, idx);
        size_t prefix_i = 1, cur_i = 0;
        for (; prefix_i < strlen(argv[0]); prefix_i++) {
            while (cur_i < strlen(cur) && cur[cur_i] == ' ') 
                cur_i++;
            if (cur_i >= strlen(cur) || argv[0][prefix_i] != cur[cur_i++]) {
                break;
            }
            if (prefix_i == strlen(argv[0]) - 1) {
                print_command(cur);
                char **new_argv;
                int res = command_exec(cur, &new_argv);
                destroy_argv(new_argv);
                new_argv = NULL;
                add_history = 0;
                return res;
            }
        }
    }
    print_no_history_match();
    add_history = 0;
    return 1;
}

int handle_ps_command() {
    rewrite_information();
    print_process_info_header();
    for (size_t i = 0; i < vector_size(vec_information); i++) {
        print_process_info(vector_get(vec_information, i));
    }
    return 0;
}

int exec_buildin(char* command, int argc, char** argv) {
    if (argc == 0) {
        print_invalid_command(command);
        return 1;
        // invalid build-in command
    }

    if (!strcmp(argv[0], "cd")) {
        add_history = 1;
        return handle_cd_command(argc, argv);
    } 
    else if (!strcmp(argv[0], "!history")) {
        add_history = 0;
        return handle_history_command();
    } 
    else if (argv[0][0] == '#') {
        // add_history = 0;
        return handle_call_num_command(argv);
    } 
    else if (argv[0][0] == '!') {
        // add_history = 0;
        return handle_latest_similar_command(argv);
    } 
    else if (!strcmp(argv[0], "exit")) {
        add_history = 0;
        processes_kill();
        shell_destroy();
        exit(0);
    } 
    // pt2
    else if (!strcmp(argv[0], "ps")) {
        add_history = 1;
        return handle_ps_command();
    }
    else if ((!strcmp(argv[0], "cont")) && (!strcmp(argv[0], "stop")) && (!strcmp(argv[0], "kill"))) {
        add_history = 1;
        if (argc == 1) {
            print_invalid_command(command);
            return 1;
        }
        //update vec_information to remove the dead process
        rewrite_information();
        int pid = atoi(argv[1]);
        process_info* it_info = NULL;
        size_t i = 0;
        for (; i < vector_size(vec_information); i++) {
            it_info = vector_get(vec_information, i);
            if (it_info->pid == pid) {
                make_doing_check(argv, command, pid);
                break;
            }
        }
        if (i == vector_size(vec_information)) {
            print_no_process_found(pid);
        }
        return 0;
    }
    else {
        return 2;  
        // only command above are build-in, others not
    }
}

void make_doing_check(char** argv, char* command, int pid) {
    if ((!strcmp(argv[0], "cont"))) {
        if (kill(pid, SIGCONT) == 0) {
            print_continued_process(pid, command);
        }
    }
    else if (!strcmp(argv[0], "stop")) {
        if (kill(pid, SIGKILL) == 0) {
            print_killed_process(pid, command);
        }
    }
    else {
        if (kill(pid, SIGSTOP) == 0) {
            print_stopped_process(pid, command);
        }
    }
    return;
}

int check_background(int argc, char** argv) {
    if (argc < 2) {
        return 0;
    }
    
    if (!strcmp(argv[argc - 1], "&")) {
        // background command, delete &
        free(argv[argc - 1]);
        argv[argc - 1] = NULL;
        return 1;
    }
    return 0;
}

// external
int make_fork(char* command, int argc, char** argv) {
    fflush(stdout);
    // clear the stdout buffer
    int isBackground = check_background(argc, argv);
    pid_t new_pid = fork();

    if (new_pid == -1) {
        print_fork_failed();
        processes_kill();
        shell_destroy();
        exit(1);
    }

    write_information(new_pid, command);
    if (new_pid && !isBackground) {
        build_fore_p(new_pid, command);
    }
    add_history = 1;

    if (new_pid == 0) {
        return run_child(command, argv, isBackground);
    } else {
        return run_parent(new_pid, isBackground);
    }
}

void build_fore_p(pid_t pid, const char* command) {
    new_fore_p = malloc(sizeof(process));
    new_fore_p->pid = pid;
    new_fore_p->command = strdup(command);
}

void free_foreground_process() {
    if (new_fore_p == NULL) {
        return;
    }
    free(new_fore_p->command);
    free(new_fore_p);
    new_fore_p = NULL;
}

int run_child(const char* command, char** argv, int isBackground) {
    pid_t process_id = getpid();
    print_command_executed(process_id);
    // prints the command being executed, along with the child's process ID.

    if (command_output) {
        // set in func command_exec()
        if (dup2(fileno(command_output), STDOUT_FILENO) == -1) {
            // redirect the stdout into file command_output
            print_redirection_file_error();
            fclose(command_output);
            command_output = NULL;
            return 1;
        }
        fclose(command_output);
        command_output = NULL;
    }

    if (isBackground && setpgid(process_id, getppid()) == -1) {
        print_setpgid_failed();
        processes_kill();
        shell_destroy();
        exit(1);
    }
    int ret = execvp(argv[0], argv);
    // if execvp failed, then exec below
    if (ret == -1) {
        print_exec_failed(command);
        // processes_kill();
        shell_destroy();
        exit(1);
    }
    return 0;
}

int run_parent(pid_t child, int isBackground) {
    int status = 0;
    if (isBackground) {
        waitpid(child, &status, WNOHANG);
        // run background, don't need to wait for child
        return 0;
    }

    if (waitpid(child, &status, 0) == -1) {
        print_wait_failed();
        processes_kill();
        shell_destroy();
        exit(1);
    }
    
    if (WIFEXITED(status) && (WEXITSTATUS(status) != EXIT_SUCCESS)) {
        //  child process terminated normally but did not return success
        free_foreground_process();
        return 1;
    }
    
    if (!WIFEXITED(status) && !WIFSIGNALED(status)) {
        // child process did not end because of receiving a signal
        processes_kill();
        shell_destroy();
        exit(1);
    }
    free_foreground_process();
    return 0;
}

void write_information(int pid, char* command_line) {
    if (pid == 0) 
        return;
    char new_file[PATHMAXSIZE];
    snprintf(new_file, sizeof(new_file), "/proc/%d/stat", pid);

	FILE* get_open_file = fopen(new_file, "r");
    if (get_open_file != NULL) {
        char state = 0;
        unsigned long utime = 0, stime = 0, vsize = 0;
        long nthreads = 0;
        unsigned long long starttime = 0;

        fscanf(get_open_file, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %ld %*ld %llu %lu" , &state, &utime, &stime, &nthreads, &starttime, &vsize);
        fclose(get_open_file);
        
        //get the boot time
        unsigned long long set_root_time = 0;
        get_open_file = popen("awk '/btime/ {print $2}' /proc/stat", "r");
        // Uses awk to search for the line containing btime 
        // in /proc/stat and directly prints the second field (which is the boot time).
        fscanf(get_open_file, "%*s %llu", &set_root_time);
        fclose(get_open_file);

        // Converting vsize to KB
        vsize = vsize / 1024;
        unsigned long long get_sysconf = sysconf(_SC_CLK_TCK);

        size_t sec = starttime / get_sysconf + set_root_time;
        size_t min = sec / 60;
        size_t h = (min / 60) % 24;
        min = min % 60;
        char* start_str = malloc(PATHMAXSIZE);
        execution_time_to_string(start_str, PATHMAXSIZE, h, min);
        
        sec = (utime + stime) / get_sysconf;
        min = sec / 60;
        sec = sec % 60;
        char* time_str = malloc(PATHMAXSIZE);
        execution_time_to_string(time_str, PATHMAXSIZE, min, sec);

        size_t len_command = strlen(command_line);
        char* command = calloc(len_command + 1, 1);
        strcpy(command, command_line);

        int i = len_command - 1;
        while (i >= 0 && command[i] == ' ') {
            command[i] = '\0';
            i--;
        }

        process_info* new_info_p = NULL;
        //search vec_information for process with same pid
        size_t v_size = vector_size(vec_information);
        size_t j = 0;
        while (j < v_size) {
            if (((process_info*)vector_get(vec_information, j))->pid == pid) {
                new_info_p = vector_get(vec_information, j);
                free(new_info_p->start_str);
                free(new_info_p->time_str);
                free(new_info_p->command);
                break;
            }
            j++;
        }
        //add new_info_p to vec_information if no match existing pid
        if (!new_info_p) {
            new_info_p = malloc(sizeof(process_info));
            vector_push_back(vec_information, new_info_p);
        }
        
        new_info_p->pid = pid;
        new_info_p->nthreads = nthreads;
        new_info_p->vsize = vsize;
        new_info_p->state = state;
        new_info_p->start_str = start_str;
        new_info_p->time_str = time_str;
        new_info_p->command = command;
        return;
    }
    print_no_process_found(pid);
    return;
}

void rewrite_information() {
    process_info* new_info_p = NULL;
    size_t i = 0;
    for (; i < vector_size(vec_information); i++) {
        new_info_p = vector_get(vec_information, i);

        char new_file[PATHMAXSIZE];
        sprintf(new_file, "/proc/%d/stat", new_info_p->pid);
        FILE *get_open_file = fopen(new_file, "r");
        if (!get_open_file) {
            //need to remove this new_info_p
            free(new_info_p->start_str);
            free(new_info_p->time_str);
            free(new_info_p->command);
            free(new_info_p);
            vector_erase(vec_information, i);
            i--;
            continue;
        }

        char state = 0;
        unsigned long utime = 0, stime = 0, vsize = 0;
        long nthreads = 0;

        fscanf(get_open_file, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %ld %*ld %*llu %lu", &state, &utime, &stime, &nthreads, &vsize);
        fclose(get_open_file);
        //remove new_info_p if get "X"
        if (state == 'X') {
            free(new_info_p->start_str);
            free(new_info_p->time_str);
            free(new_info_p->command);
            free(new_info_p);
            vector_erase(vec_information, i);
            i--;
            continue;
        }
        vsize = vsize / 1024;
        size_t sec = (utime + stime) / CLOCKS_PER_SEC;
        size_t min = sec / 60;
        sec = sec % 60;
        char* time_str = malloc(PATHMAXSIZE);
        execution_time_to_string(time_str, PATHMAXSIZE, min, sec);
        
        //rewrite information
        new_info_p->vsize = vsize;
        new_info_p->state = state;
        new_info_p->nthreads = nthreads;
        
        free(new_info_p->time_str);
        new_info_p->time_str = time_str;
    }
}

void destroy_argv(char** input) {
    if (input) {
        for (size_t i = 0;input[i] != NULL;i ++) {
            free(input[i]);
            input[i] = NULL;
        }
        free(input);
        input = NULL;
    }
}

void shell_destroy() {
    if (file_name && history_command) {
        // only -h
        FILE* store_history = fopen(file_name, "w");
        for (size_t i = 0; i < vector_size(history_command); i++) {
            char* file_write = (char*)vector_get(history_command, i);
            fprintf(store_history, "%s\n", file_write);
        }
        // after shell end, write history into file
        fclose(store_history);
        free(file_name);
    }
    if (history_command) {
        vector_destroy(history_command);
    }

    if (vec_information) {
        size_t i = 0;
        for (; i < vector_size(vec_information); i++) {
            process_info* new_info = vector_get(vec_information, i);
            //free memory
            free(new_info->start_str);
            free(new_info->time_str);
            free(new_info->command);
            free(new_info);
        }
        vector_destroy(vec_information);
    }
    if (new_fore_p) {
        pid_t cur_pid = new_fore_p->pid;
        char* cur_command = new_fore_p->command;
        if (kill(cur_pid, SIGINT) == -1) {
            print_no_process_found(cur_pid);
        }
        else {
            print_killed_process(cur_pid, cur_command);
        }
        free(new_fore_p->command);
        free(new_fore_p);
        new_fore_p = NULL;
    }
    
    if (command_output) {
        fclose(command_output);
    }
    if (command_words) {
        vector_destroy(command_words);
    }
    if (cur_dir) { 
        free(cur_dir); 
        cur_dir = NULL; 
    }
    if (first_command) { 
        free(first_command); 
        first_command = NULL;
    }
    if (second_command) { 
        free(second_command); 
        second_command = NULL; 
    }
    
    destroy_argv(old_argv);
}

void processes_kill() {
    if (vec_information == NULL) {
        return;
    }
    for (size_t i = 0; i < vector_size(vec_information); i++) {
        process_info* cur = vector_get(vec_information, i);
        if (cur->pid == getpid()) {
            continue;
        }
        if (cur->state == 'X') {     
            // exit_dead: exit state, process is about to be destroyed
            continue;
        }
        kill(cur->pid, SIGKILL);
    }
}