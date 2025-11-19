#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mysh.h"

const char *bultin_commands[] = {"cd", "pwd", "which", "exit", "die", NULL};

static char *executable_path(const char *command) {
    if (strchr(command, '/')) {
        char *path = malloc(strlen(command) + 1);

        if(!path) return NULL; // Memory allocation failed

        strcpy(path, command);

        // Command contains a slash, treat as a path
        if (access(command, X_OK) == 0) {
            return path;
        }
        
        else {
            free(path);
            return NULL;
        }
    }

    const char *default_dirs[] = {"/usr/local/bin", "/usr/bin", "/bin", NULL}; // Default directories
    for (int i = 0; default_dirs[i] != NULL; i++) {
        int len = strlen(default_dirs[i]) + strlen(command) + 2; // +2 for '/' and null terminator
        char *full_path = malloc(len); // Allocate memory for full path

        if(!full_path) return NULL; // Memory allocation failed

        snprintf(full_path, len, "%s/%s", default_dirs[i], command);

        if (access(full_path, X_OK) == 0) {
            return full_path;
        }

        free(full_path);
    }

    return NULL; // Command not found
}

int is_builtin(Command *cmd) {
    if (!cmd || cmd->argc == 0) {
        return 0; // Not a built-in if command is NULL or has no arguments
    }
    for (int i = 0; bultin_commands[i] != NULL; i++) {
        if (strcmp(cmd->argv[0], bultin_commands[i]) == 0) {
            return 1; // Command is a built-in
        }
    }

    return 0; // Command is not a built-in
}

// Helper function to handle I/O redirection
static void redirect_io(Command *cmd) {
    if (cmd->input_redir != NULL) {
        int input_fd = open(cmd->input_redir, O_RDONLY); // Open input file for reading

        if (input_fd < 0) {
            perror("open input redirection");
            exit(EXIT_FAILURE);
        }

        dup2(input_fd, STDIN_FILENO); // Redirect stdin to input file
        close(input_fd);
    }

    if (cmd->output_redir != NULL) {
        int output_fd = open(cmd->output_redir, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open output file for writing

        if (output_fd < 0) {
            perror("open output redirection");
            exit(EXIT_FAILURE);
        }

        dup2(output_fd, STDOUT_FILENO); // Redirect stdout to output file
        close(output_fd);
    }
}

// Execute a built-in command
int execute_builtin(Command *cmd, int parent_stdout) {
    /* for (int i = 0; bultin_commands[i] != NULL; i++) {
        if (strcmp(cmd->argv[0], bultin_commands[i]) == 0) {
            // Handle cd command
            if (strcmp(cmd->argv[0], "cd") == 0) {
                if(parent_stdout) return 0; // Do nothing if in a subshell

                if (cmd->argv[1] == NULL) {
                    fprintf(stderr, "cd: missing argument\n");
                    return 1;
                }

                if (chdir(cmd->argv[1]) != 0) {
                    perror("cd");
                    return 1;
                }

                return 0;
            } 

            // Handle cwd command
            if(strcmp(cmd->argv[0], "cwd") == 0) {
                if(parent_stdout) return 0; // Do nothing if in a subshell

                char cwd[MAX_PATH];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("%s\n", cwd);
                } 
                
                else {
                    perror("getcwd");
                    return 1;
                }

                return 0;
            }
            
            // Handle exit/die commands
            else if (strcmp(cmd->argv[0], "exit") == 0 || strcmp(cmd->argv[0], "die") == 0) {
                if(!parent_stdout) {
                    printf("mysh: exiting\n");
                    fflush(stdout);
                }
                
                if (strcmp(cmd->argv[0], "exit") == 0)
                    exit(0);          // main shell exits successfully
                
                else
                    exit(EXIT_FAILURE); // main shell exits with failure
            }
        }

        return 0; // Built-in command executed
    }

    return 1; // Unknown built-in command*/

    if (!cmd || !cmd->argv[0]) return 1;
    const char *name = cmd->argv[0];
    // cd 
    if (strcmp(name, "cd") == 0) {
        if (!parent_stdout) return 0; // subshell: do nothing and succeed 
        if (cmd->argc != 2) { fprintf(stderr, "cd: wrong number of arguments\n"); return 1; }
        if (chdir(cmd->argv[1]) != 0) { perror("cd"); return 1; }
        return 0;
    }
    // pwd
    if (strcmp(name, "pwd") == 0) {
        char cwd[MAX_PATH];
        if (getcwd(cwd, sizeof(cwd)) == NULL) { perror("pwd"); return 1; }
        printf("%s\n", cwd);
        return 0;
    }
    // which 
    if (strcmp(name, "which") == 0) {
        if (cmd->argc != 2) { fprintf(stderr, "which: wrong number of arguments\n"); return 1; }
        // builtin names cause which to fail
        const char *builtin_list[] = {"cd","pwd","which","exit","die", NULL};
        for (int i = 0; builtin_list[i]; i++) if (strcmp(cmd->argv[1], builtin_list[i]) == 0) return 1;
        char *p = executable_path(cmd->argv[1]);
        if (!p) return 1;
        printf("%s\n", p);
        free(p);
        return 0;
    }
    // exit + die 
    if (strcmp(name, "exit") == 0 || strcmp(name, "die") == 0) {
        if (!parent_stdout) {
            // in child: just exit 
            if (strcmp(name, "exit") == 0) exit(EXIT_SUCCESS);
            else {
                if (cmd->argc > 1) {
                    for (int k = 1; k < cmd->argc; k++) {
                        if (k > 1) fprintf(stderr, " ");
                        fprintf(stderr, "%s", cmd->argv[k]);
                    }
                    fprintf(stderr, "\n");
                }
                exit(EXIT_FAILURE);
            }
        } else {
            // parent: request shell to exit 
            extern int shell_signal_exit;
            extern int shell_exit_status;
            if (strcmp(name, "exit") == 0) {
                shell_signal_exit = 1;
                shell_exit_status = EXIT_SUCCESS;
            } else {
                if (cmd->argc > 1) {
                    for (int k = 1; k < cmd->argc; k++) {
                        if (k > 1) fprintf(stderr, " ");
                        fprintf(stderr, "%s", cmd->argv[k]);
                    }
                    fprintf(stderr, "\n");
                }
                shell_signal_exit = 1;
                shell_exit_status = EXIT_FAILURE;
            }
            return 0;
        }
    }
    return 1; // unknown 
}

// Helper function to set up pipes for a command in a pipeline
static void pipe_handler(int index, int num_commands, int pipe_fds[MAX_COMMANDS*2]) {
    if (index > 0) {
        // Not the first command, redirect stdin to read end of previous pipe
        dup2(pipe_fds[(index - 1) * 2], STDIN_FILENO);
    }

    if (index < num_commands - 1) {
        // Not the last command, redirect stdout to write end of current pipe
        dup2(pipe_fds[index * 2 + 1], STDOUT_FILENO);
    }
}

// Helper function to close all pipe file descriptors
static void close_pipes(int num_commands, int pipe_fds[MAX_COMMANDS*2]) {
    for (int i = 0; i < (num_commands - 1) * 2; i++) {
        close(pipe_fds[i]);
    }
}

int execute_job(Job *job, int parent_stdout) {
    int num_commands = job->num_commands;
    if (num_commands > MAX_COMMANDS) {
        fprintf(stderr, "Error: too many commands in pipeline (max %d)\n", MAX_COMMANDS);
        return 1;
    }

    extern int shell_signal_exit;
    extern int shell_exit_status;

    int pipe_fds[MAX_COMMANDS*2];  // Fixed-size array for pipes

    // Create pipes for inter-process communication
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipe_fds + i * 2) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

        // execute only one built in command in parent, tepmp use dup2()
    if (num_commands == 1 && is_builtin(job->commands[0])) {
        Command *c = job->commands[0];
        int saved_in = -1, saved_out = -1;
        if (c->input_redir) {
            int infd = open(c->input_redir, O_RDONLY);
            if (infd < 0) { 
                perror(c->input_redir); 
                return 1; 
            }
            saved_in = dup(STDIN_FILENO);
            dup2(infd, STDIN_FILENO);
            close(infd);
        }
        if (c->output_redir) {
            int outfd = open(c->output_redir, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (outfd < 0) { 
                perror(c->output_redir); 
                if (saved_in != -1) { 
                    dup2(saved_in, STDIN_FILENO); 
                    close(saved_in);
                } 
                return 1; 
            }
            saved_out = dup(STDOUT_FILENO);
            dup2(outfd, STDOUT_FILENO);
            close(outfd);
        }
        int r = execute_builtin(c, 1);
        if (saved_in != -1) { 
            dup2(saved_in, STDIN_FILENO); 
            close(saved_in); 
        }
        if (saved_out != -1) { dup2(saved_out, STDOUT_FILENO); close(saved_out); }
        // if builtin set shell_signal_exit. main will exit, 0 or 1 for sucess or fail
        return (r == 0) ? 0 : 1;
    }

    // Execute each command in the job
    for (int i = 0; i < num_commands; i++) {
        Command *cmd = job->commands[i];

        if (num_commands == 1 && is_builtin(cmd) && parent_stdout) {
            // Execute built-in command in the parent process if it's the only command
            return execute_builtin(cmd, 1);
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // Child process
        if (pid == 0) {
            pipe_handler(i, num_commands, pipe_fds);
            redirect_io(cmd);

            if (is_builtin(cmd)) {
                execute_builtin(cmd, parent_stdout || num_commands > 1);
                exit(0);
            }
            
            else {
                char *exec_path = executable_path(cmd->argv[0]);

                if (exec_path == NULL) {
                    fprintf(stderr, "%s: command not found\n", cmd->argv[0]);
                    exit(EXIT_FAILURE);
                }
                
                execv(exec_path, cmd->argv);
                perror("execv");
                free(exec_path);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Parent process
    close_pipes(num_commands, pipe_fds);

    // Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }

    return 0; // Job executed successfully
}
