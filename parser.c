#include "mysh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_line(char *line, struct Job *job){
    memset(job, 0, sizeof(*job)); // Initialize job structure

    char *tokens[TOKEN_SIZE];
    int position_token = 0;

    char *tok = strtok(line, " \t\r\n");
    while(tok && position_token < TOKEN_SIZE - 1){
        if(*tok == '#'){
            break; // Ignore comments
        }
        
        tokens[position_token++] = tok;
        tok = strtok(NULL, " \t\r\n");
    }

    tokens[position_token] = NULL;

    if(position_token == 0){
        return; // Empty line
    }

    // Parse commands
    if(strcmp(tokens[0], "and") == 0 || strcmp(tokens[0], "or") == 0){
        strncpy(job->operator, tokens[0], sizeof(job->operator) - 1);
        job->operator[sizeof(job->operator) - 1] = '\0';
    } 
    
    else {
        job->operator[0] = '\0'; // No operator
    }

    // Create first command
    Command *cmd = malloc(sizeof(Command));
    cmd->argc = 0;
    cmd->input_redir = NULL;
    cmd->output_redir = NULL;

    // Parse command arguments and redirections
    for(int i = 0; i < position_token; i++){
        if(strcmp(tokens[i], "<") == 0){
            if(i + 1 < position_token){
                cmd->input_redir = strdup(tokens[++i]);
            }

            else {
                // Handle error: no file specified for input redirection
                fprintf(stderr, "Error: No input file specified for redirection\n");
                free(cmd);
                return;
            }
        } 
        
        else if(strcmp(tokens[i], ">") == 0){
            if(i + 1 < position_token){
                cmd->output_redir = strdup(tokens[++i]);
            }

            else {
                // Handle error: no file specified for output redirection
                fprintf(stderr, "Error: No output file specified for redirection\n");
                free(cmd);
                return;
            }
        }

        else if(strcmp(tokens[i], '|') == 0){
            cmd->argv[cmd->argc] = NULL; // Null-terminate current command arguments
            job->commands[job->num_commands++] = cmd; // Add command to job

            // Create new command for the next segment
            cmd = malloc(sizeof(Command));
            cmd->argc = 0;
            cmd->input_redir = NULL;
            cmd->output_redir = NULL;
        }
        
        else {
            cmd->argv[cmd->argc++] = strdup(tokens[i]);
        }
    }

    // Add the last command
    if(cmd->argc > 0){
        cmd->argv[cmd->argc] = NULL; // Null-terminate last command arguments
        job->commands[job->num_commands++] = cmd;
    }
    
    else {
        free(cmd); // Free if no arguments were added
    }
}

void free_job(struct Job *job){
    for(int i = 0; i < job->num_commands; i++){
        Command *cmd = job->commands[i];

        if(!cmd) continue;

        for(int j = 0; j < cmd->argc; j++){
            free(cmd->argv[j]); // Free each argument
        }

        free(cmd->input_redir); // Free input redirection file
        free(cmd->output_redir); // Free output redirection file
        free(cmd); // Free command structure
        job->commands[i] = NULL; // Clear command pointer
    }

    job->num_commands = 0; // Reset job command count
    job->operator[0] = '\0'; // Clear operator
}