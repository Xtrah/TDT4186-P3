#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

int exit_flag = 0;

void redirectIn(char *filename) {
    // Redirect stdin to first argument after < 
    int inFd = open(filename, O_RDONLY);
    if (inFd < 0) {
        printf("ERROR: Could not open file %s\n", filename);
        return;
    }
    dup2(inFd, STDIN_FILENO);
    close(inFd);
}

void redirectOut(char *filename) {
    // Redirect stdout to first argument after > 
    int outFd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (outFd < 0) {
        printf("ERROR: Could not open file %s\n", filename);
        return;
    }
    dup2(outFd, STDOUT_FILENO);
    close(outFd);
}

// Write prompt to screen and return input
void prompt(char *input) {
    size_t path_size = 100;
    char* path = malloc(sizeof(char) * path_size); // Allocate memory for path
    getcwd(path, path_size); // Get working directory
    printf("%s: ", path); // Print working directory
    free(path); // Free memory
    
    // Takes input from command line
    char buffer[1024];
    
    if (buffer == fgets(buffer, sizeof(buffer), stdin)) { 
        buffer[strcspn(buffer, "\r\n")] = 0; // Remove newline from input
        strcpy(input, buffer);
    }
    // Exits if fgets fails
    else if (strcmp(buffer, "\0") == 0) {
        // TODO: 3.1 NOT COMPLETED: Check for EOF instead
        exit_flag = 1;
        printf("EOF\n");
    }
}

// Parse input and puts args in array separated by space or tab
void parse_arguments(char **args, char *input) {
    int i = 0;
    char *token[256];
    
    token[i] = strtok(input, " \t"); // Split input by space or tab
    while (token[i] != NULL) {
        // Redirect stdin
        if (strcmp(token[i], "<") == 0) {
            //if (token[i+1]) {
                char *tok = strtok(NULL, " \t");
                printf("%s\n", tok);
                redirectIn(tok);
                i++;
            // }
            // else {
            //     printf("ERROR: No such file (redirectIn)\n");
            //     exit(EXIT_FAILURE);
            // }
        }
        // Redirect stdout
        else if (strcmp(token[i], ">") == 0) {
            //if (token[i+1]) {
                char *tok = strtok(NULL, " \t");
                printf("%s\n", tok);
                redirectOut(tok);
                i++;
            // }
            // else {
            //     printf("ERROR: No such file (redirectOut)\n");
            //     exit(EXIT_FAILURE);
            // }
        }
        else {
            args[i] = token[i]; // Put token in args array
            i++;
            token[i] = strtok(NULL, " \t"); 
        }
    }
}

void execute(char **args) {
    // Checks for change directory
    if (strcmp(args[0], "cd") == 0) {
        if (args[2] == NULL) { // Only two arguments allowed
            if (chdir(args[1]) < 0) printf("ERROR: %s\n", strerror(errno));
            return;
        }
        printf("Too many arguments!\n");
    }

    pid_t pid = fork();

    // Child process
    if (pid == 0) {
        int rc = execvp(args[0], args);
        if (rc < 0) printf("ERROR: %s\n", strerror(errno));
    }
    // Parent process
    else if (pid > 0) {
        int status;
        pid_t child = waitpid(pid, &status, 0);
        printf("Exit status [%s] = %d\n", args[0], status);
    }

    // Reset stdin and stdout
    redirectIn("/dev/tty");
    redirectOut("/dev/tty");
}

// head -1 < /tmp/ex.txt > /tmp/foo.txt

// freopen(args[1], "r", stdin);
// TODO: Lag funksjon som redirecter stdin stdout
// - Sjekke etter piler <, >:
//   - sette stdin stdout med freopen
//   - execute hver kommando
//   - stdin, stdout kan bare settes én gang
//   - hvis redirect ikke eksister, kan kommandoen executes vanlig
// - Tokenize etter <, > først? Deretter tokenize argumenter?
// - Les hele kommando fra høyre til venstre?


int main() {
    while(!exit_flag) {
        char *input = malloc(sizeof(char) * 1024);
        prompt(input);
        printf("Input: %s\n", input);

        char **args = malloc(sizeof(char) * 1024);
        parse_arguments(args, input);

        // Ignores if first argument (command) is empty
        if (args[0] == NULL) {
            continue;
        }

        execute(args);

        free(args);
        free(input);
    }
    exit(EXIT_SUCCESS);
}