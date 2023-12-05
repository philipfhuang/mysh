#ifndef __BUILTINS_H__
#define __BUILTINS_H__

#include <unistd.h>
#include <netinet/in.h>

#include "variables.h"
#include "io_helpers.h"


/* Type for builtin handling functions
 * Input: Array of tokens
 * Return: >=0 on success and -1 on error
 */
struct listen_sock {
    struct sockaddr_in addr;
    int sock_fd;
};

struct client_sock {
    int sock_fd;
    char buf[MAX_STR_LEN];
    int inbuf;
    struct client_sock *next;
};

extern int is_server_started;
extern int server_pid;
typedef ssize_t (*bn_ptr)(char **);
ssize_t bn_echo(char **tokens);
ssize_t bn_cd(char** path);
ssize_t bn_cat(char** path);
ssize_t bn_wc(char** path);
ssize_t bn_ls(char** tokens);
ssize_t bn_ps(char** tokens);
ssize_t bn_kill(char** tokens);
ssize_t bn_start_server(char** tokens);
ssize_t bn_close_server(char** tokens);
ssize_t bn_send(char** tokens);
ssize_t bn_start_client(char** tokens);



/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd);
void pipeline(char** tokens);
ssize_t execute(char** tokens);

/* BUILTINS and BUILTINS_FN are parallel arrays of length BUILTINS_COUNT
 */
static const char * const BUILTINS[] = {"echo", "=", "cd", "cat", "wc", "ls", "ps", "kill", "start-server",
                                        "close-server", "send", "start-client"};
static const bn_ptr BUILTINS_FN[] = {bn_echo, makeNode, bn_cd, bn_cat, bn_wc, bn_ls, bn_ps, bn_kill, bn_start_server,
                                     bn_close_server, bn_send, bn_start_client, NULL};    // Extra null element for 'non-builtin'
static const size_t BUILTINS_COUNT = sizeof(BUILTINS) / sizeof(char *);

#endif
