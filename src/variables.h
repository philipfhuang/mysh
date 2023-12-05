#ifndef __VARIABLES_H__
#define __VARIABLES_H__

typedef struct Node{
    char* name;
    char* value;
    struct Node *next;
}Node;

extern Node* environment;

ssize_t makeNode(char **tokens);
char* getValue(char* name);

#endif