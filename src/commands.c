#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "commands.h"
#include "builtins.h"
#include "other_helpers.h"
#include "io_helpers.h"

Element* cmd = NULL;
int cmds_count = 1;
int finished_count = 0;

void store_cmd(char** command, int pid, int count) {
    if (cmd == NULL) {
        cmd = malloc(sizeof(Element));
        cmd->name = malloc(sizeof(char) * strlen(command[0]) + 1);
        strcpy(cmd->name, command[0]);
        cmd->command = malloc(sizeof(char) * MAX_STR_LEN);
        cmd->command[0] = '\0';
        int i = 0;
        if (command[i] != NULL) {
            strcat(cmd->command, command[i]);
            i++;
        }
        while (command[i] != NULL) {
            strcat(cmd->command," ");
            strcat(cmd->command, command[i]);
            i++;
        }
        cmd->pid = pid;
        cmd->next = NULL;
        cmd->count = cmds_count;
    } else {
        Element* curr = cmd;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = malloc(sizeof(Element));
        curr->next->name = malloc(sizeof(char) * strlen(command[0]) + 1);
        strcpy(curr->next->name, command[0]);
        curr->next->command = malloc(sizeof(char) * MAX_STR_LEN);
        curr->next->command[0] = '\0';
        int i = 0;
        if (command[i] != NULL) {
            strcat(curr->next->command, command[i]);
            i++;
        }
        while (command[i] != NULL) {
            strcat(curr->next->command," ");
            strcat(curr->next->command, command[i]);
            i++;
        }
        curr->next->pid = pid;
        curr->next->next = NULL;
        curr->next->count = cmds_count;
    }
    cmds_count++;
}

void run_background(char** tokens, int count) {
    int r;
    tokens[count - 1] = NULL;
    if ((r = fork()) > 0) {
        char buf[10];
        sprintf(buf, "%d", cmds_count);
        display_message("[");
        display_message(buf);
        display_message("] ");
        sprintf(buf, "%d", r);
        display_message(buf);
        display_message("\n");

        store_cmd(tokens, r, count);
    } else if (r == 0) {
        if (!is_pipe(tokens)) {
            bn_ptr builtin_fn = check_builtin(tokens[0]);
            if (builtin_fn != NULL) {
                ssize_t err = builtin_fn(tokens);
                if (err == - 1) {
                    display_error("ERROR: Builtin failed: ", tokens[0]);
                }
            } else {
                ssize_t err = execute(tokens);
                if (err == - 1) {
                    display_error("ERROR: Unrecognized command: ", tokens[0]);
                }
            }
        } else {
            pipeline(tokens);
        }
        exit(0);
    } else {
        display_error("ERROR: System call fails", "fork");
    }

}

void check_finish(void) {
    Element* prev = NULL;
    Element* curr = cmd;
    while (curr != NULL) {
        int status;
        if (waitpid((pid_t)curr->pid, &status, WNOHANG) != 0) {
            char buf[10];
            sprintf(buf, "%d", curr->count);
            display_message("[");
            display_message(buf);
            display_message("]+  Done ");
            display_message(curr->command);
            display_message("\n");

            if (prev != NULL) {
                prev->next = curr->next;
            } else {
                cmd = cmd->next;
            }

            Element* tmp = curr;
            curr = curr->next;

            free(tmp->command);
            free(tmp->name);
            free(tmp);
            finished_count++;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
    if (finished_count == cmds_count - 1) {
        cmds_count = 1;
        finished_count = 0;
    }
}

