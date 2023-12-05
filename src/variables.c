#include <stdlib.h>
#include <string.h>

#include "variables.h"

Node* environment = NULL;

ssize_t makeNode(char **tokens) {
    char* curr_ptr = strtok (tokens[0], "=");
    char* name = curr_ptr;

    char* value = curr_ptr + strlen(name) + 1;

    if (value[0] == '$') {
        char* exist = getValue(value + sizeof(char));
        if (exist) {
            value = exist;
        }
    } else {
        if (environment == NULL) {
            environment = malloc(sizeof(Node));
            environment->name = malloc(sizeof(char) * strlen(name) + 1);
            environment->value = malloc(sizeof(char) * strlen(value) + 1);
            strcpy(environment->name,name);
            strcpy(environment->value,value);
            environment->next = NULL;
        } else {
            Node* curr = environment;
            while (curr->next != NULL && strcmp(curr->name, name) != 0) {
                curr = curr->next;
            }
            if (strcmp(curr->name, name) == 0) {
                free(curr->value);
                curr->value = malloc(sizeof(char) * strlen(value) + 1);
                strcpy(curr->value,value);
            } else {
                curr->next = malloc(sizeof(Node));
                curr->next->name = malloc(sizeof(char) * strlen(name) + 1);
                curr->next->value = malloc(sizeof(char) * strlen(value) + 1);
                strcpy(curr->next->name,name);
                strcpy(curr->next->value,value);
                curr->next->next = NULL;
            }
        }
    }


    return 0;
}

char* getValue(char* name) {
    Node* curr = environment;
    while (curr != NULL){
        if (strcmp(curr->name, name) == 0) {
            return curr->value;
        }
        curr = curr->next;
    }
    return NULL;
}