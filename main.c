#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#define WCPSH_RL_BUFSIZE 1024
#define WCPSH_TOK_BUFSIZE 64
#define WCPSH_TOK_DELIM " \t\r\n\a"

int wcpsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("wcpsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("wcpsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && WIFSIGNALED(status));
    }

    return 1;
}

char **wcpsh_split_line(char *line) {
    int bufsize = WCPSH_TOK_BUFSIZE, position = 0;

    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "wcpSH: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, WCPSH_TOK_DELIM);

    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += WCPSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if (!tokens) {
                fprintf(stderr, "wcpSH: allocation error");
            }
        }

        token = strtok(NULL, WCPSH_TOK_DELIM);
    }
    tokens[position] = NULL;

    return tokens;
}

char *wcpsh_read_line(void) {
    int bufsize = WCPSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "wcpSH: allocation error\n");
    }

    while (1) {
        // Read a char
        c = getchar();

        if (c == EOF || c == "\n") {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }

        if (position >= bufsize) {
            bufsize += WCPSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize); 
            if (!buffer) {
                fprintf(stderr, "wcpSH: allocation error"); 
            }
        }
    }
}

void wcpsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("wcpSH >> ");
        line = wcpsh_read_line();
        args = wcpsh_split_line(line);
        status = wcpsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv) {
    // Load config files, if any
    //
    // Run command loop
    wcpsh_loop();
    // Perform any shutdown/cleanup

    return EXIT_SUCCESS;
}
