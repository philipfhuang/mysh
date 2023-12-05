#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "io_helpers.h"
#include "other_helpers.h"


void ls_display_f(DIR *dir, int d_val, char* f_val, char* path) {
    if (d_val < 1) {
        closedir(dir);
        return;
    }
    struct dirent *dp = readdir (dir);
    while (dp != NULL) {
        if (strstr(dp->d_name, f_val)) {
                display_message(dp->d_name);
                display_message("\n");
        }
        if ((dp->d_type == 4) && strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
            char next_path[strlen(path) + strlen(dp->d_name) + 3];
            strncpy(next_path, path, strlen(path));
            next_path[strlen(path)] = '/';
            next_path[strlen(path) + 1] = '\0';
            strncat(next_path, dp->d_name, strlen(dp->d_name));
            next_path[strlen(path) + strlen(dp->d_name) + 2] = '\0';
            ls_display_f(opendir(next_path), d_val - 1, f_val, next_path);
        }
        dp = readdir (dir);
    }
    closedir(dir);
}

void ls_display(DIR *dir, int d_val, char* path) {
    if (d_val < 1) {
        closedir(dir);
        return;
    }
    struct dirent *dp = readdir (dir);
    while (dp != NULL) {
        display_message(dp->d_name);
        display_message("\n");

        if ((dp->d_type == 4) && strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
            char next_path[strlen(path) + strlen(dp->d_name) + 3];
            strncpy(next_path, path, strlen(path));
            next_path[strlen(path)] = '/';
            next_path[strlen(path) + 1] = '\0';
            strncat(next_path, dp->d_name, strlen(dp->d_name));
            next_path[strlen(path) + strlen(dp->d_name) + 2] = '\0';
            ls_display(opendir(next_path), d_val - 1, next_path);
        }
        dp = readdir (dir);
    }
    closedir(dir);
}

int is_pipe(char** token_arr) {
    int i = 0;
    while (token_arr[i] != NULL) {
        if (strcmp(token_arr[i], "|") == 0) {
            return 1;
        }
        i++;
    }
    return 0;
}

int is_space(char c) {
    if ((c == ' ') || (c == '\n') || (c == '\t') || (c == '\v') || (c == '\f') || (c == '\r')) return 1;
    return 0;
}

int is_background(char** token_arr, int count) {
    if (strcmp(token_arr[count - 1], "&") == 0) {
        return 1;
    }
    return 0;
}

int find_network_newline(const char *buf, int inbuf) {
    for (int i = 0; i < inbuf - 1; i++) {
        if (buf[i] == '\r' && buf[i + 1] == '\n') {
            return i + 2;
        }
    }
    return -1;
}

int read_from_socket(int sock_fd, char *buf, int *inbuf) {
    int read_count = read(sock_fd, buf + *inbuf, MAX_STR_LEN - *inbuf);
    if (MAX_STR_LEN <= *inbuf || read_count == -1) {
        return -1;
    }
    *inbuf += read_count;
    for (int i = 0; i < *inbuf - 1; i++) {
        if (buf[i] == '\r' && buf[i + 1] == '\n') {
            return 0;
        }
    }
    if (read_count == 0) {
        return 1;
    }
    return 2;
}

int get_message(char **dst, char *src, int *inbuf) {
    int msg_len = find_network_newline(src, *inbuf);
    if (msg_len == -1) {
        return 1;
    }
    *dst = malloc(msg_len - 1);
    if (*dst == NULL) {
        perror("malloc");
        return 1;
    }
    memmove(*dst, src, msg_len - 2);
    (*dst)[msg_len - 2] = '\0';

    memmove(src, src + msg_len, MAX_STR_LEN - msg_len);
    *inbuf -= msg_len;
    return 0;
}

int write_to_socket(int sock_fd, char *buf, int len) {
    int error = write(sock_fd, buf, len);
    if (error == 0) return 2;
    if (error == -1) return 1;
    return 0;
}

int remove_client(struct client_sock **curr, struct client_sock **clients) {
    if (*curr == *clients) {
        *clients = (*clients)->next;
        free(*curr);
        *curr = NULL;
        return 0;
    }

    struct client_sock* curr_ptr = *clients;
    while (curr_ptr != NULL) {
        if (curr_ptr->next == *curr) {
            curr_ptr->next = (*curr)->next;
            free(*curr);
            *curr = NULL;
            return 0;
        }
        curr_ptr = curr_ptr->next;
    }

    return 1;
}