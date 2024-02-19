#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


#define MAX_INPUT 1024
#define MAX_TOKENS 80
#define MAX_ARGS 10
#define MAX_PIDS 5

void execute_command(char *tokens[], pid_t pid_history[], int *pid_count) {
    if (tokens[0] != NULL) {
        if (strcmp(tokens[0], "cd") == 0) {
            change_dir(tokens);
        } else if (strcmp(tokens[0], "showpid") == 0){
            show_pids(pid_history, *pid_count);
        } else {
            execute_externals(tokens, pid_history, pid_count);
        }
    }
}

void execute_externals(char *tokens[], pid_t pid_history[], int *pid_count) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(tokens[0], tokens) == -1) {
            fprintf(stderr, "Error: Command could not be executed\n");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("Error: ");
    } else {
        add_pids(pid, pid_history, pid_count);
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

void add_pids(pid_t pid, pid_t pid_history[], int *pid_count) {
    for (int i = MAX_PIDS - 1; i > 0; i--) {
        pid_history[i] = pid_history[i - 1];
    }
    pid_history[0] = pid;
    *pid_count = (*pid_count < MAX_PIDS) ? (*pid_count + 1) : MAX_PIDS;
}

void show_pids(pid_t pid_history[], int pid_count) {
    for (int i = 0; i < pid_count; i++) {
        printf("%d\n", pid_history[i]);
    }
}

void change_dir(char *tokens[]) {
    if (tokens[1] == NULL) {
        fprintf(stderr, "Error: Missing argument for cd\n");
    } else {
        if (chdir(tokens[1]) == 0) {
            setenv("PWD", getcwd(NULL, 0), 1);
        } else {
            perror("Error: ");
        }
    }
}

void parse_input(char *input, char *tokens[]) {
    memset(tokens, 0, MAX_ARGS * sizeof(char*));

    int i = 0;
    char *token = strtok(input, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1) {
        tokens[i] = malloc(MAX_TOKENS);
        strncpy(tokens[i], token, MAX_TOKENS - 1);
        tokens[i][MAX_TOKENS - 1] = '\0';
        i++;
        token = strtok(NULL, " \t\n");
    }
}

void free_tokens(char *tokens[]) {
    for (int i = 0; i < MAX_ARGS; i++) {
        free(tokens[i]);
    }
}

int main() {
    char input[MAX_INPUT];
    char *tokens[MAX_ARGS];
    pid_t pid_history[MAX_PIDS];
    int pid_count = 0;

    while (1) {
        printf("o %s$ ", getcwd(NULL, 0));
        fgets(input, sizeof(input), stdin);
        if (strcmp(input, "exit\n") == 0) {
            printf("Exiting shell\n");
            break;
        }
        parse_input(input, tokens);

        execute_command(tokens, pid_history, &pid_count);

        free_tokens(tokens);
    }

    return 0;
}
