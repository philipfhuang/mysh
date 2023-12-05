#ifndef __OTHER_HELPERS_H__
#define __OTHER_HELPERS_H__

#include <dirent.h>
#include <string.h>

#include "builtins.h"

void ls_display(DIR *dir, int d_val, char* path);
void ls_display_f(DIR *dir, int d_val, char* f_val, char* path);

int is_pipe(char** token_arr);
int is_space(char c);
int is_background(char** token_arr, int count);

int find_network_newline(const char *buf, int n);
int read_from_socket(int sock_fd, char *buf, int *inbuf);
int get_message(char **dst, char *src, int *inbuf);
int write_to_socket(int sock_fd, char *buf, int len);
int remove_client(struct client_sock **curr, struct client_sock **clients);

#endif