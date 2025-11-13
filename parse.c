#include "mysh.h"

void parse_line(char *line, struct Job *job){
    memset(job, 0, sizeof(*job)); // Initialize job structure

    char *tokens[TOKEN_SIZE];
    int position_token = 0;

    char *token = strtok(line, " \t\r\n");
    while(token && position_token < TOKEN_SIZE - 1){
        if(*token == '#'){
            break; // Ignore comments
        }
        
        token[position_token++] = token;
        token = strtok(NULL, " \t\r\n");
    }

    tokens[position_token] = NULL;

    if(position_token == 0){
        return; // Empty line
    }
}