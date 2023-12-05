#ifndef __COMMANDS_H__
#define __COMMANDS_H__

typedef struct Element{
    char* name;
    char* command;
    int pid;
    struct Element *next;
    int count;
}Element;

extern Element* cmd;
extern int count;
extern int finished_count;

void run_background(char** tokens, int count);
void check_finish(void);

#endif