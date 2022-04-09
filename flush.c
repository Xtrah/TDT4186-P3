#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "linkedlist.h"

int exit_flag = 0;
int background_flag = 0;

List *bg_processes;

/**
 * @brief Redirects stdin to a file
 * 
 * @param filename Name of file to redirect to
 */
int redirectIn(char *filename) {
    int inFd = open(filename, O_RDONLY);
    if (inFd < 0) {
        fprintf(stderr, "ERROR: %s: '%s'\n", strerror(errno), filename);
        return -1;
    }
    dup2(inFd, STDIN_FILENO);
    return close(inFd);
}

/**
 * @brief Redirects stdout to a file
 * 
 * @param filename Name of file to redirect to
 */
int redirectOut(char *filename) {
    int outFd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (outFd < 0) {
        fprintf(stderr, "ERROR: %s: '%s'\n", strerror(errno), filename);
        return -1;
    }
    dup2(outFd, STDOUT_FILENO);
    return close(outFd);
}

/**
 * @brief Write prompt to screen and get input from user
 * 
 * @param input Return string containing user input
 */
void prompt(char *input) {
    size_t path_size = 100;
    char* path = malloc(sizeof(char) * path_size); // Allocate memory for path
    getcwd(path, path_size); // Get working directory
    printf("%s: ", path); // Print working directory
    free(path);
    
    // Takes input from command line
    char buffer[1024];
    
    if (buffer == fgets(buffer, sizeof(buffer), stdin)) { 
        buffer[strcspn(buffer, "\r\n")] = 0; // Remove newline from input
        strcpy(input, buffer);
    }
    // Exits if fgets fails
    // else if (strcmp(buffer, "\0") == 0) {
    //     // TODO: 3.1 NOT COMPLETED: Check for EOF instead
    //     exit_flag = 1;
    //     printf("EOF\n");
    // }
}

/**
 * @brief Parse input and puts args in array separated by space or tab
 * 
 * @param args Return list of parsed arguments
 * @param input String input to be parsed
 */
void parse_arguments(char **args, char *input) {
    // TODO: Format input with spaces around special characters.

    int i = 0;
    char *token = strtok(input, " \t"); // Split input by space or tab
    
    while (token) {
        // Redirect stdin
        if (strcmp(token, "<") == 0) {
            char *tok = strtok(NULL, " \t");
            int rc = redirectIn(tok);
            if (rc < 0) break;
        }
        // Redirect stdout
        else if (strcmp(token, ">") == 0) {
            char *tok = strtok(NULL, " \t");
            int rc = redirectOut(tok);
            if (rc < 0) break;
        }
        // Sets task to background if & is provided
        else if (strcmp(token, "&") == 0) {
            background_flag = 1;
            break; // No more args allowed after &
        }
        else {
            args[i] = token; // Put token in args array
            i++;
        }
        token = strtok(NULL, " \t"); 
    }

    args[i] = NULL;
}

void print_exit_status(char *command, int status) {
    printf("Exit status [%s] = %d\n", command, status);
}

/**
 * @brief Executes provided arguments in child process
 * 
 * @param args Arguments to be executed
 */
void execute(char **args) {
    // Ignores if first argument (i.e. command) is empty
    if (args[0] == NULL) {
        return;
    }

    // Change directory command
    if (strcmp(args[0], "cd") == 0) {
        if (args[2] == NULL) { // Only two arguments allowed
            if (chdir(args[1]) < 0) printf("ERROR: %s\n", strerror(errno));
            return;
        }
        printf("ERROR: Too many arguments!\n");
    }

    // Exit command
    if (strcmp(args[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    }

    if (strcmp(args[0], "jobs") == 0) {
        printf("---- CURRENT JOBS: ----\n");
        display(bg_processes);
        return;
    }

    pid_t pid = fork();

    // Child process
    if (pid == 0) {
        int rc = execvp(args[0], args);
        if (rc < 0) printf("ERROR: %s\n", strerror(errno));
    }
    // Parent process
    else if (pid > 0) {
        // Reset stdin and stdout
        redirectIn("/dev/tty");
        redirectOut("/dev/tty");

        int status = 0;


        char command[1024];
        strcpy(command, args[0]);
        int i = 1;
        while (args[i] != NULL) {
            strcat(command, " ");
            strcat(command, args[i]);
            i++;
        }

        // Check if process should be run in background
        if (!background_flag) {  
            waitpid(pid, &status, 0);
            print_exit_status(command, status);
        }
        else {
            add(pid, command, bg_processes);
            background_flag = 0;
        }
    }
}

int main() {
    bg_processes = makelist();
    while(!exit_flag) {
        pid_t pid;
        do {
            int status;
            pid = waitpid(-1, &status, WNOHANG);

            if (pid > 0) {
                Node *node = findnode(bg_processes, pid);
                print_exit_status(getcommand(node), status);
                delete(pid, bg_processes);
            }
        } while(pid > 0);

        char *input = malloc(sizeof(char) * 1024);
        prompt(input);

        char **args = malloc(sizeof(char) * 1024);
        parse_arguments(args, input);

        execute(args);

        free(args);
        free(input);
    }
    free(bg_processes);
    exit(EXIT_SUCCESS);
}