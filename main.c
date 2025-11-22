#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

// Function to read the command line
char *read_line(void);

// Function to split the line into tokens
char **split_line(char *line);

// Function to execute the command
int execute(char **args);

void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    char *line;
    char **args;
    int status = 1;

    printf("Starting simple C shell...\n");

    // Set up the SIGCHLD handler
    signal(SIGCHLD, sigchld_handler);

    do {
        // 1. READ
        printf("myshell> ");
        line = read_line();

        // 2. PARSE
        args = split_line(line);
        
        // 3. EXECUTE
        status = execute(args);

        // Clean up memory
        free(line);
        if (args) {
            // Free all argument pointers before freeing the array itself
            // NOTE: More robust memory cleanup may be required depending on implementation.
            for (int i = 0; args[i] != NULL; i++) {
                free(args[i]);
            }
            free(args);
        }

    } while (status);

    return EXIT_SUCCESS;
}

// --- Implementation Stubs ---

// STUB 1: Implement the logic to read the user's input line
char *read_line(void) {
    // Implement using getline() or fgets()
    char *line = NULL;
    size_t bufsize = 0;
    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS); // Handle EOF (Ctrl+D)
        } else {
            perror("myshell: getline error");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

// STUB 2: Implement the logic to tokenize the input line

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"

char **split_line(char *line) {
    // Implement using strtok() to separate tokens by space
    // and store them in a dynamically allocated char **
    // Remember to terminate the array with a NULL pointer!
    // For now, return a placeholder:
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        perror("myshell: allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position] = strdup(token);
        position++;

        if (position >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                perror("myshell: allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

// STUB 3: Implement the fork, execvp, and wait logic
int execute(char **args) {
    // Built-in exit command
    if (args[0] == NULL) {
        return 1; // Empty command
    }
   // Built-in exit/quit
    if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0)
        return 0;

    // Process execution (fork and execvp) goes here!
    pid_t pid;//, wpid;
    int status;

    // Background job check
    int background = 0;
    int last = 0;
    while (args[last] != NULL) last++;

    if (last > 0 && strcmp(args[last - 1], "&") == 0) {
        background = 1;
        free(args[last - 1]);
        args[last - 1] = NULL;  // Remove &
    } else {
        if (!background) {
            // Foreground: wait
            if (waitpid(pid, &status, 0) == -1)
                perror("waitpid");
        } else {
            // Background: do NOT wait
            printf("[background pid %d]\n", pid);
        }
    }

    // Pipe detection
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;  // split the command

            char **left = args;
            char **right = &args[i+1];

            int pipefd[2];
            pipe(pipefd);

            pid_t p1 = fork();
            if (p1 == 0) {
                // Left child
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                execvp(left[0], left);
                perror("execvp left");
                exit(1);
            }

            pid_t p2 = fork();
            if (p2 == 0) {
                // Right child
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[1]);
                execvp(right[0], right);
                perror("execvp right");
                exit(1);
            }

            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(p1, NULL, 0);
            waitpid(p2, NULL, 0);
            return 1;
        }
    }

    pid = fork();

    if (pid == 0) {
        // Output redirection: command > file
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], ">") == 0) {
                int fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("open"); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);

                args[i] = NULL;  // terminate args before '>'
                break;
            }
        }

        // Input redirection: command < file
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "<") == 0) {
                int fd = open(args[i+1], O_RDONLY);
                if (fd < 0) { perror("open"); exit(1); }
                dup2(fd, STDIN_FILENO);
                close(fd);

                args[i] = NULL;  
                break;
            }
        }

        // Child process: execute the command
        // NOTE: Need to strip newline from the last argument before execvp
        if (execvp(args[0], args) == -1) {
            perror("myshell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("myshell: fork error");
    } else {
        // Parent process: wait for child
        do {
           if (waitpid(pid, &status, 0) == -1) {
            perror("myshell: waitpid error");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1; // Continue the loop
}
