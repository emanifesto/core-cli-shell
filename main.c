#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

// Function to read the command line
char *read_line(void);

// Function to split the line into tokens
char **split_line(char *line);

// Function to execute the command
int execute(char **args);

int main() {
    char *line;
    char **args;
    int status = 1;

    printf("Starting simple C shell...\n");

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
char **split_line(char *line) {
    // Implement using strtok() to separate tokens by space
    // and store them in a dynamically allocated char **
    // Remember to terminate the array with a NULL pointer!
    // For now, return a placeholder:
    char **tokens = malloc(sizeof(char *) * 2);
    if (!tokens) { perror("malloc failed"); exit(EXIT_FAILURE); }
    tokens[0] = strdup(line); // Simplified: first token is the whole line
    tokens[1] = NULL;
    return tokens;
}

// STUB 3: Implement the fork, execvp, and wait logic
int execute(char **args) {
    // Built-in exit command
    if (args[0] == NULL) {
        return 1; // Empty command
    }
    if (strcmp(args[0], "exit\n") == 0) { // Note: may need to strip newline
        return 0; // Signal main loop to terminate
    }

    // Process execution (fork and execvp) goes here!
    pid_t pid;//, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process: execute the command
        // NOTE: Need to strip newline from the last argument before execvp
        if (execvp(args[0], args) == -1) {
            perror("myshell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("myshell");
    } else {
        // Parent process: wait for child
        do {
            //wpid = waitpid(pid, &status, 0); // Blocking wait
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1; // Continue the loop
}
