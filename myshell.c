#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define MAX_ARGS 100
#define MAX_HISTORY 100

char *history[MAX_HISTORY];
int history_count = 0;
pid_t child_pid = -1;

void run_command(char *command);

// Ctrl+C handler 
void sigint_handler(int sig) {
    if (child_pid > 0) {
        kill(child_pid, SIGINT); //kill child
    } else {
        printf("\nsh> ");
        fflush(stdout);
    }
}

// Save  history
void add_to_history(char *cmd) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    }
}

// Display history
void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

// Break command into words
void split_input(char *command, char **args) {
    int i = 0;
    args[i] = strtok(command, " \t\n");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \t\n");
    }
}

// Handle I/O: < > >>
void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            // printf("Redirecting output to file: %s\n", args[i+1]);
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
            args[i + 1] = NULL;
            
        } else if (strcmp(args[i], ">>") == 0) {
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
            args[i + 1] = NULL;
            
        } else if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
            args[i + 1] = NULL;
        }
    }
}

// Execute piped commands like ls | grep txt
void execute_pipes(char **commands, int n) {
    int fd[2];
    int input_fd = 0;

    for (int i = 0; i < n; i++) {
        pipe(fd);
        if (fork() == 0) {
            dup2(input_fd, 0); //read end
            if (i < n - 1) {
                dup2(fd[1], 1); // write end 
            }
            close(fd[0]);

            char *args[MAX_ARGS];
            split_input(commands[i], args);
            handle_redirection(args);
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }
        close(fd[1]);
        input_fd = fd[0];
    }

    for (int i = 0; i < n; i++) {
        wait(NULL);
    }
}

// Handle multiple commands separated by ;
void run_sequence(char *line) {
    char *cmd = strtok(line, ";");
    while (cmd != NULL) {
        run_command(cmd);
        cmd = strtok(NULL, ";");
    }
}

// Handle commands separated by &&
void run_conditional(char *line) {
    char *cmd = strtok(line, "&&");
    while (cmd != NULL) {
        pid_t pid = fork();
        if (pid == 0) {
            char *args[MAX_ARGS];
            split_input(cmd, args);
            handle_redirection(args);
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else {
            int status;
            waitpid(pid, &status, 0);

           
            if (status != 0) {
                break; // break if the previous command failed
            }
        }
        cmd = strtok(NULL, "&&");
    }
}

// Main function 
void run_command(char *command) {
    while (*command == ' ') command++; // Skip spaces

    if (strlen(command) == 0)
        return;

    if (strstr(command, "&&")) {
        run_conditional(command);
    } else if (strstr(command, ";")) {
        run_sequence(command);
    } else if (strchr(command, '|')) {
        char *pipe_cmds[MAX_ARGS];
        int i = 0;
        pipe_cmds[i] = strtok(command, "|");
        while (pipe_cmds[i] != NULL) {
            i++;
            pipe_cmds[i] = strtok(NULL, "|");
        }
        execute_pipes(pipe_cmds, i);
    } else {
        pid_t pid = fork();
        child_pid = pid;

        if (pid == 0) {
            char *args[MAX_ARGS];
            split_input(command, args);
            //printf("Running command: %s\n", args[0]);
            if (args[0] == NULL)
                exit(0);

            if (strcmp(args[0], "history") == 0) {
                print_history();
                exit(0);
            }

            handle_redirection(args);
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            child_pid = -1;
        } else {
            perror("fork");
        }
    }
}

// main loop of the shell
int main() {
    char line[BUFFER_SIZE];

    signal(SIGINT, sigint_handler); // Ctrl+C handler

    while (1) {
        printf("sh> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            // printf("User: %s\n", line);
            printf("\n");
            break; //  (Ctrl+D)
        }

        line[strcspn(line, "\n")] = '\0'; // Remove newline

        if (strlen(line) == 0)
            continue;

        if (strcmp(line, "exit") == 0)
            break;

        add_to_history(line);
        run_command(line);
    }

    return 0;
}
