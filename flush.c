#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Write prompt to screen and return input
void prompt(char *input) {
    size_t path_size = 100;
    char* path = malloc(sizeof(char) * path_size);
    getcwd(path, path_size);
    printf("%s: ", path);
    free(path);
    
    // TODO: Fix newline at end of input.
    char buffer[1024];
    if (buffer == fgets(buffer, sizeof(buffer), stdin)) {
        int ret = sscanf(buffer, "%s", input);

        if (ret == 1) {
            printf("buffer: %s\n input: %s\n", buffer, input);
        }
    }
}

// Parse input and puts args in array separated by space or tab
void parse(char **args, char *input) {
    int i = 0;
    char *token[256];
    token[i] = strtok(input, " \t");
    while (token[i] != NULL) {
        args[i] = token[i];
        i++;
        token[i] = strtok(NULL, " \t");
    }
}

int main() {
    while(1) {
        char *input = malloc(sizeof(char) * 1024);
        prompt(input);

        char **args = malloc(sizeof(char) * 1024);
        parse(args, input);

        // int i = 0;
        // while(args[i] != NULL) {
        //     printf("%s,", args[i]);
        //     i++;
        // }

        if (args[0] == NULL) {
            printf("Invalid command\n");
            continue;
        }

        pid_t pid = fork();

        // Child process
        if (pid == 0) {
            int rc = execv(args[0], args);
            if (rc < 0) printf("ERROR\n");
        }
        // Parent process
        else if (pid > 0) {
            int status;
            pid_t child = waitpid(pid, &status, 0);
            printf("Exit status [%s] = %d\n", args[0], status);
        }
    }
}