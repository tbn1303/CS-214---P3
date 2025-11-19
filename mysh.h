#ifndef MYSH_H
#define MYSH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>

#define BUFSIZE 128 // Buffer size for reading lines
#define TOKEN_SIZE 128 // Size for tokenizing lines
#define ARGS 64 // Maximum number of arguments
#define MAX_PATH 4096 // Maximum path length
#define MAX_COMMANDS 16 // Maximum number of commands in a job

typedef struct Command {
    char *argv[ARGS];      // Argument list
    char *input_redir;     // Input redirection file
    char *output_redir;    // Output redirection file
    int argc;              // Argument count
} Command;

typedef struct Job {
    Command *commands[ARGS]; // List of commands in the job
    int num_commands;        // Number of commands
    char operator[4];      // Operator such as "and", "or" or empty
} Job;

extern int shell_signal_exit; // Flag to indicate shell should exit
extern int shell_exit_status; // Exit status to return when shell exits

void parse_line(char *line, Job *job); // Parse input line into a Job structure
void free_job(Job *job); // Free memory allocated for a Job structure
int execute_job(Job *job, int prev_status); // Execute the job
int is_builtin(Command *cmd); // Check if command is a built-in
int execute_builtin(Command *cmd, int parent_stdout); // Execute built-in command

#endif
