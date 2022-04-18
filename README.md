# TDT4186 P3 Unix shell

In this project, we will create a simple Unix shell to perform some basic tasks such as changing between directories (`cd`) and execution of commands.

Additionally, the program also handles special tasks such as I/O redirection (`<`, `>`), background tasks (`&`, `jobs`) and pipelines (`|`).

> To compile and run the shell, execute the `build.sh` file.

## 3.1 Basic functionality

The `prompt()` function prompts the user with the current path and reads a line of input provided by the user using `fgets()`. After, if `feof()` detects that the user has pressed `Ctrl-D`, an exit flag will be set to signal to the program that it should exit as soon as possible.

After the user has provided input, this is passed on to `parse_arguments()`. First, `format_input()` is used on the input string, surrounding all special characters with whitespace to guarantee correct tokenization later. Tokenization is then done with `strtok()`, adding each argument into an array, while also checking for special characters and acting accordingly.

When all arguments are parsed, the array is passed to `execute()` which runs the command. First, it checks for special commands, and if none are found it creates a child process using `fork()` in which the arguments are executed. If a normal command is executed the parent process will wait for the child to complete and print the exit status of the command.

## 3.2 Changing directories

To change the current working directory, we use the `cd` command. This command takes one argument, which is the path to the new working directory. `cd` works just like it would in a normal shell, such as bash, zsh and PowerShell.

The check for the `cd` command is done in `execute()` using the `chdir()` syscall to change the current directory. Then `execute()` returns to continue to the next prompt.

`cd` has to be an internal shell command built into the shell so that the shell doesn't have to spawn a new process to call the command. Everything happens without calling any PATH variables.

## 3.3 I/O redirection

Redirection of input and output can be done using the `<` and `>` symbols. The `<` symbol redirects the input to a file, and the `>` symbol redirects the output to a file.

In `parse_arguments()` we implemented an `if`-check that looks for `<` and `>` in the input.

If the input contains `>`, we redirect the output of a command from the left into the right using `redirect_out()`.

If the input contains `<`, we redirect the input to the right into the left using `redirect_in()`:

Example from code `flush.c:208-213`:

```c
// Redirect stdin
if (strcmp(token, "<") == 0) {
    char *tok = strtok(NULL, " \t");
    int rc = redirect_in(tok);
    if (rc < 0) break;
}
```

## 3.4 Background tasks

Background tasks are tasks that are executed in the background, and do not block the execution of the shell. By adding the `&` symbol to the end of a command, the command is executed in the background by setting background_flag = 1 in `parse_arguments()`. This signals to `execute()` that the parent process should not wait for the child to complete, but rather put the process info into the `bg_processes` linked list.

We store the PID of the background tasks in a linked list imported from `linkedlist.h`. The linked list is inspired by [skorks/c-linked-list](https://github.com/skorks/c-linked-list) but modified to fit our program. PIDs are stored in the order they are executed.

Before any command is executed, we check for zombie processes and print their exit status. This happens in `main()`.

## 3.5 Background task status

To view the status of background tasks we implemented the `jobs` command. The command takes no extra arguments.

In `execute()` the program checks for the `jobs` command and runs the linked list's `display()` function to write out a list of all background processes recorded in the `bg_processes` list.

## 3.6 Pipelines

We implemented pipelines, which lets us execute a sequence of commands connected by the `|` character. The commands are executed in the order they are given. The result of the last command is the input for the next command.

Pipes are created during the parsing of arguments when detcting the `|` symbol. `create_pipe()` then uses the syscall `pipe()` to create file descriptors for the read and write parts of the pipe. The currently accumulated arguments are then executed with `stdout` redirected to the pipe. After, `stdin` is redirected to the pipe, making the next command take its input from the pipe.
