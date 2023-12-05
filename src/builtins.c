#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <dirent.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include "builtins.h"
#include "io_helpers.h"
#include "other_helpers.h"
#include "commands.h"

int is_server_started = 0;
int server_pid = -1;

// ====== Command execution =====

/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd) {
    ssize_t cmd_num = 0;
    if (strchr(cmd, '=') != NULL) {
        return BUILTINS_FN[1];
    }
    while (cmd_num < BUILTINS_COUNT &&
           (strncmp(BUILTINS[cmd_num], cmd, MAX_STR_LEN) != 0)) {
        cmd_num += 1;
    }
    return BUILTINS_FN[cmd_num];
}


// ===== Builtins =====

/* Prereq: tokens is a NULL terminated sequence of strings.
 * Return 0 on success and -1 on error ... but there are no errors on echo. 
 */
ssize_t bn_echo(char **tokens) {
    ssize_t index = 1;

    if (tokens[index] != NULL) {
        // TODO:
        // Implement the echo command
        display_message(tokens[index]);
        index += 1;
    }
    while (tokens[index] != NULL) {
        // TODO:
        // Implement the echo command
        display_message(" ");
        display_message(tokens[index]);
        index += 1;

    }
    display_message("\n");

    return 0;
}

ssize_t bn_cd(char** path) {
    ssize_t err = chdir(path[1]);
    if (err == -1) {
        display_error("ERROR: ", "Invalid path");
        return -1;
    }
    return err;
}

ssize_t bn_cat(char** path) {
    int open_stdin = 0;
    FILE* f = fopen(path[1], "r");
    if (path[1] == NULL) {
        struct pollfd fds;
        fds.fd = 0;
        fds.events = POLLIN;
        int ret = poll(&fds, 1, 10);
        if (ret == 0) {
            display_error("ERROR: ", "No input source provided");
            return -1;
        } else {
            f = stdin;
            open_stdin++;
        }
    } else if (f == NULL) {
        display_error("ERROR: ", "Cannot open file");
        return -1;
    }
    char buf[2];
    buf[1] = '\0';
    while (fread(buf, sizeof(char), 1, f)) {
        display_message(buf);
    }

    if (!open_stdin) {
        fclose(f);
    }
    return 0;
}

ssize_t bn_wc(char** path) {
    int open_stdin = 0;

    int chars = 0;
    int words = 0;
    int lines = 0;
    char buf;

    FILE* f = fopen(path[1], "r");
    if (path[1] == NULL) {
        struct pollfd fds;
        fds.fd = 0;
        fds.events = POLLIN;
        int ret = poll(&fds, 1, 10);
        if (ret == 0) {
            display_error("ERROR: ", "No input source provided");
            return -1;
        } else {
            f = stdin;
            open_stdin++;
        }
    } else if (f == NULL) {
        display_error("ERROR: ", "Cannot open file");
        return -1;
    }

    int is_word = 0;

    while (fread(&buf, sizeof(char), 1, f)) {
        chars++;
        if (!is_space(buf)) {
            if (!is_word) {
                is_word = 1;
                words++;
            }
        } else {
            if (buf == '\n') {
                lines++;
            }
            if (is_word) {
                is_word = 0;
            }
        }
    }

    char counts[30];
    counts[9] = '\0';
    sprintf(counts, "word count %d", words);
    display_message(counts);
    display_message("\n");

    sprintf(counts, "character count %d", chars);
    display_message(counts);
    display_message("\n");

    sprintf(counts, "newline count %d", lines);
    display_message(counts);
    display_message("\n");

    if (!open_stdin) {
        fclose(f);
    }
    return 0;
}

ssize_t bn_ls(char** tokens) {
    DIR *dir;

    int i = 1;
    int is_path = 0;
    int is_f = 0;
    int is_rec = 0;
    int is_d = 0;

    char* f_val;
    char* path;
    int d_val = 1;

    while (tokens[i] != NULL) {
        if (strcmp(tokens[i], "--f") == 0) {
            if (is_f) {
                closedir(dir);
                return -1;
            }
            is_f = 1;
            f_val = tokens[i + 1];
            i++;
        } else if (strcmp(tokens[i], "--rec") == 0) {
            if (is_rec) {
                closedir(dir);
                return -1;
            }
            is_rec = 1;
        } else if (strcmp(tokens[i], "--d") == 0) {
            if (is_d) {
                closedir(dir);
                display_message("bad d");
                return -1;
            }
            is_d = 1;
            d_val = atoi(tokens[i + 1]);
            i++;
        } else {
            if (is_path) {
                closedir(dir);
                return -1;
            }
            is_path = 1;
            path = tokens[i];
            dir = opendir (tokens[i]);
        }
        i++;
    }

    if (is_rec != is_d) {
        return -1;
    }

    if (!is_path) {
        path = ".";
        dir = opendir (".");
    }

    if (dir == NULL) {
        display_error("ERROR: ", "Invalid path");
        return -1;
    }

    if (is_f) {
        ls_display_f(dir, d_val, f_val, path);
    } else {
        ls_display(dir, d_val, path);
    }
    return 0;
}

ssize_t bn_ps(char** tokens) {
    Element* curr = cmd;
    while (curr != NULL) {
        display_message(curr->name);
        display_message(" ");
        char buf[10];
        sprintf(buf, "%d", curr->pid);
        display_message(buf);
        display_message("\n");
        curr = curr->next;
    }
    return 0;
}

ssize_t bn_kill(char** tokens) {
    pid_t pid = atol(tokens[1]);
    int sig;
    if (tokens[2] == NULL) {
        sig = SIGTERM;
    } else {
        sig = atoi(tokens[2]);
    }

    int err = kill(pid, sig);

    if (errno == EINVAL) {
        display_error("ERROR: ", "Invalid signal specified");
    } else if (errno == ESRCH) {
        display_error("ERROR: ", "The process does not exist");
    }
    return err;
}

ssize_t bn_start_server(char** tokens) {
    if (tokens[1] == NULL) {
        display_error("ERROR: ", "No port provided");
        return -1;
    }
    if (is_server_started) {
        display_error("ERROR: ", "Server started already");
        return -1;
    }

    int port_num = atoi(tokens[1]);

    struct listen_sock s;
    s.sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    s.addr.sin_family = AF_INET;
    s.addr.sin_port = htons(port_num);
    memset(&s.addr.sin_zero, 0, 8);
    s.addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s.sock_fd, (struct sockaddr *) &(s.addr), sizeof(struct sockaddr_in)) < 0) {
        display_error("ERROR: System call failed: ", "bind");
        close(s.sock_fd);
        return -1;
    }

    if (listen(s.sock_fd, 10) < 0) {
        display_error("ERROR: System call failed: ", "listen");
        close(s.sock_fd);
        return -1;
    }

    is_server_started = 1;

    server_pid = fork();
    if (server_pid == -1) {
        is_server_started = 0;
        display_error("ERROR: System call failed: ", "fork");
        return -1;
    }
    if (server_pid == 0) {
        struct client_sock *clients = NULL;

        fd_set all_fds, listen_fds;
        FD_ZERO(&all_fds);
        FD_SET(s.sock_fd, &all_fds);

        int max_fd = s.sock_fd;

        do {
            listen_fds = all_fds;
            if (select(max_fd + 1, &listen_fds, NULL, NULL, NULL) == -1) {
                display_error("ERROR: System call failed: ", "select");
                exit(1);
            }
            if (FD_ISSET(s.sock_fd, &listen_fds)) {
                struct sockaddr_in peer;
                unsigned int peer_len = sizeof(peer);
                peer.sin_family = AF_INET;

                struct client_sock *curr = clients;
                int client_fd = accept(s.sock_fd, (struct sockaddr *)&peer, &peer_len);
                if (client_fd < 0) {
                    display_error("ERROR: ", "Failed to accept incoming connection, system call: accept may be failed");
                    continue;
                }
                if (client_fd > max_fd) {
                    max_fd = client_fd;
                }
                FD_SET(client_fd, &all_fds);

                struct client_sock *newclient = malloc(sizeof(struct client_sock));
                newclient->sock_fd = client_fd;
                newclient->inbuf = 0;
                newclient->next = NULL;
                memset(newclient->buf, 0, MAX_STR_LEN);
                if (clients == NULL) {
                    clients = newclient;
                }
                else {
                    curr->next = newclient;
                }
            }

            struct client_sock *curr = clients;
            while (curr) {
                if (!FD_ISSET(curr->sock_fd, &listen_fds)) {
                    curr = curr->next;
                    continue;
                }
                int client_closed = read_from_socket(curr->sock_fd, curr->buf, &(curr->inbuf));

                char *msg;
                while (client_closed == 0 && !get_message(&msg, curr->buf, &(curr->inbuf))) {
                    display_message("\n");
                    display_message(msg);
                    display_message("\nmysh$ ");
                    free(msg);
                }

                if (client_closed == -1) {
                    FD_CLR(curr->sock_fd, &all_fds);
                    close(curr->sock_fd);
                    if (remove_client(&curr, &clients)) {
                        display_error("ERROR: ", "Problem with removing client");
                        exit(1);
                    }
                }
                else {
                    curr = curr->next;
                }
            }
        } while (is_server_started);
        exit(0);
    }
    return 0;
}

ssize_t bn_close_server(char** tokens) {
    if (server_pid == -1 || !is_server_started) {
        display_error("ERROR: ", "server is not started");
        return -1;
    }
    if (kill(server_pid, 9) == -1) {
        display_error("ERROR: System call failed: ", "kill");
        return -1;
    }
    is_server_started = 0;
    return 0;
}

ssize_t bn_send(char** tokens) {
    char* rest = NULL;
    int port_num = strtol(tokens[1], &rest, 0);
    if (rest[0] != '\0') {
        display_error("ERROR: ", "No port provided");
        return -1;
    }

    if (tokens[2] == NULL) {
        display_error("ERROR: ", "No hostname provided");
        return -1;
    }

    int soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc == -1) {
        display_error("ERROR: System call failed:", " socket");
        return -1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port_num);
    memset(&server.sin_zero, 0, 8);

    struct addrinfo *ai;
    char * hostname = tokens[2];
    getaddrinfo(hostname, NULL, NULL, &ai);
    server.sin_addr = ((struct sockaddr_in *) ai->ai_addr)->sin_addr;
    freeaddrinfo(ai);

    if (connect(soc, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
        display_error("ERROR: System call failed:", " connect");
        return -1;
    }

    char msg[MAX_STR_LEN + 3];
    msg[0] = '\0';
    int i = 3;
    if (tokens[i] != NULL) {
        strcat(msg, tokens[i]);
        i++;
    }
    while (tokens[i] != NULL) {
        strcat(msg, " ");
        strcat(msg, tokens[i]);
        i++;
    }

    int msg_len = strlen(msg);
    msg[msg_len] = '\r';
    msg[msg_len + 1] = '\n';

    if (write(soc, msg, msg_len + 2) == -1){
        display_error("ERROR: System call failed:", " write");
        return -1;
    }
    close(soc);
    return 0;
}

ssize_t bn_start_client(char** tokens) {
    if (tokens[1] == NULL) {
        display_error("ERROR: ", "No port provided");
        return -1;
    }

    char* rest = NULL;
    int port_num = strtol(tokens[1], &rest, 0);
    if (rest[0] != '\0') {
        display_error("ERROR: ", "No port provided");
        return -1;
    }

    if (tokens[2] == NULL) {
        display_error("ERROR: ", "No hostname provided");
        return -1;
    }

    int soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc == -1) {
        display_error("ERROR: System call failed:", " socket");
        return -1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port_num);
    memset(&server.sin_zero, 0, 8);

    struct addrinfo *ai;
    char * hostname = tokens[2];
    getaddrinfo(hostname, NULL, NULL, &ai);
    server.sin_addr = ((struct sockaddr_in *) ai->ai_addr)->sin_addr;
    freeaddrinfo(ai);

    if (connect(soc, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
        display_error("ERROR: System call failed:", " connect");
        return -1;
    }

    char msg[MAX_STR_LEN + 3];

    while (1) {
        int read_len = read(STDIN_FILENO, msg, MAX_STR_LEN + 1);
        if (read_len == -1 || read_len == 0) {
            close(soc);
            break;
        }
        if (read_len > MAX_STR_LEN) {
            write(STDERR_FILENO, "ERROR: input line too long\n", strlen("ERROR: input line too long\n"));
            int junk = 0;
            while((junk = getchar()) != EOF && junk != '\n');
            continue;
        }
        msg[read_len - 1] = '\r';
        msg[read_len] = '\n';
        if (write(soc, msg, read_len + 1) == -1){
            display_error("ERROR: System call failed:", " write");
            return -1;
        }
    }
    return 0;
}

void pipeline(char** tokens) {
    int fork_err;
    if ((fork_err = fork()) > 0) {
        if (wait(NULL) == -1) {
            display_error("ERROR: System call fails ", "wait");
        }
    } else if (fork_err == 0) {
        char* prev_tokens[MAX_STR_LEN] = {NULL};
        char* rest_tokens[MAX_STR_LEN] = {NULL};
        int count = 0;
        while (strcmp(tokens[count], "|") != 0) {
            prev_tokens[count] = tokens[count];
            count++;
        }
        prev_tokens[count] = NULL;
        count++;
        int i = 0;
        while (tokens[count] != NULL) {
            rest_tokens[i] = tokens[count];
            i++;
            count++;
        }
        rest_tokens[i] = NULL;

        int fd[2], r;
        if ((pipe(fd)) == -1) {
            display_error("ERROR: System call fails", "pipe");
            exit(1);
        }
        if ((r = fork()) > 0) {

            if ((dup2(fd[1], STDOUT_FILENO)) == -1) {
                display_error("ERROR: System call fails", "dup2");
                exit(1);
            }

            if ((close(fd[0])) == -1) {
                display_error("ERROR: System call fails", "close");
            }

            if ((close(fd[1])) == -1) {
                display_error("ERROR: System call fails", "close");
            }

            bn_ptr builtin_fn = check_builtin(prev_tokens[0]);
            if (builtin_fn != NULL) {
                ssize_t err = builtin_fn(prev_tokens);
                if (err == -1) {
                    display_error("ERROR: Builtin failed: ", prev_tokens[0]);
                }
            } else {
                ssize_t err = execute(prev_tokens);
                if (err == - 1) {
                    display_error("ERROR: Unrecognized command: ", prev_tokens[0]);
                }
            }

            if (fclose(stdout) == -1) {
                display_error("ERROR: System call fails", "fclose");
                exit(1);
            }

            if(wait(NULL) == -1) {
                display_error("ERROR: System call fails", "wait");
            }
        } else if (r == 0) {
            if ((dup2(fd[0], STDIN_FILENO)) == -1) {
                perror("dup2");
                exit(1);
            }

            if ((close(fd[1])) == -1) {
                perror("close");
            }

            if ((close(fd[0])) == -1) {
                perror("close");
            }

            if (is_pipe(rest_tokens)) {
                pipeline(rest_tokens);
            } else {
                bn_ptr builtin_fn = check_builtin(rest_tokens[0]);
                if (builtin_fn != NULL) {
                    ssize_t err = builtin_fn(rest_tokens);
                    if (err == -1) {
                        display_error("ERROR: Builtin failed: ", rest_tokens[0]);
                    }
                } else {
                    ssize_t err = execute(rest_tokens);
                    if (err == - 1) {
                        display_error("ERROR: Unrecognized command: ", rest_tokens[0]);
                    }
                }
            }
            exit(0);
        } else {
            display_error("ERROR: System call fails", "fork");
            exit(1);
        }
        exit(0);
    } else {
        display_error("ERROR: System call fails ", "fork");
    }
}

ssize_t execute(char** tokens) {
    int fork_err, status;
    if ((fork_err = fork()) > 0) {
        if (wait(&status) == -1) {
            display_error("ERROR: System call fails ", "wait");
        }
        if(WEXITSTATUS(status) == 1) {
            return -1;
        }
        return 0;
    } else if (fork_err == 0) {
        char path_bin[MAX_STR_LEN + 6] = "/bin/";
        strncat(path_bin, tokens[0], strlen(tokens[0]));
        execv(path_bin, tokens);

        char path_usr_bin[MAX_STR_LEN + 10] = "/usr/bin/";
        strncat(path_usr_bin, tokens[0], strlen(tokens[0]));
        execv(path_usr_bin, tokens);
        exit(1);
    } else {
        display_error("ERROR: System call fails ", "fork");
    }
    return -1;
}