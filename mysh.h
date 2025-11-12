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

#define BUFSIZE 1024 // Buffer size for reading lines
#define TOKEN_BUFSIZE 64 // Buffer size for tokenizing lines
#define ARGS 128 // Maximum number of arguments

typedef struct Command {
    char **argv[ARGS];      // Argument list
    char *input_redir;     // Input redirection file
    char *output_redir;    // Output redirection file
    int argc;              // Argument count
} Command;

typedef struct {
    Command *commands[ARGS]; // List of commands in the job
    int num_commands;        // Number of commands
    char operator[4];      // Operator such as "|", "&&", "||"
} Job;

int parse_line(char *line, Job *job); // Parse input line into a Job structure
int execute_job(Job *job, int prev_status); // Execute the job
int is_builtin(const char *cmd); // Check if command is a built-in
int execute_builtin(Command *cmd); // Execute built-in command

#endif
