#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "builtins.h"
#include "io_helpers.h"
#include "variables.h"
#include "other_helpers.h"
#include "commands.h"


void handler(int sig) {
    display_message("\n");
}

int main(int argc, char* argv[]) {
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    char *prompt = "mysh$ "; // TODO Step 1, Uncomment this.

    char input_buf[MAX_STR_LEN + 1];
    input_buf[MAX_STR_LEN] = '\0';
    char *token_arr[MAX_STR_LEN] = {NULL};

    while (1) {
        // Prompt and input tokenization

        // TODO Step 2:
        // Display the prompt via the display_message function.
        display_message(prompt);

        int ret = get_input(input_buf);
        size_t token_count = tokenize_input(input_buf, token_arr);

        check_finish();

        if (token_count == 0 && ret != 0) {
            continue;
        }

        // Clean exit
        if (ret != -1 && (token_count == 0 || (strncmp("exit", token_arr[0], 5) == 0))) {
            break;
        }

        // Command execution
        if (token_count >= 1) {
            if (is_background(token_arr, token_count)) {
                run_background(token_arr, (int)token_count);
            } else if (!is_pipe(token_arr)) {
                bn_ptr builtin_fn = check_builtin(token_arr[0]);
                if (builtin_fn != NULL) {
                    ssize_t err = builtin_fn(token_arr);
                    if (err == - 1) {
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                    }
                } else {
                    ssize_t err = execute(token_arr);
                    if (err == - 1) {
                        display_error("ERROR: Unrecognized command: ", token_arr[0]);
                    }
                }
            } else {
                pipeline(token_arr);
            }
        }

    }
    if (is_server_started) {
        char** holder = NULL;
        bn_close_server(holder);
    }
    return 0;
}
