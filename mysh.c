#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "mysh.h"

#define PROMPT "mysh> "

int interactive_mode = 0; // Global flag for interactive mode

void signal_handler(int signo) {
    if (interactive_mode) {
        write(STDOUT_FILENO, "\n", 1);
        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
    }
}

//Structure to handle line reading from file descriptor
typedef struct{
    char *buf;
    int fd;
    int pos;
    int bytes;
}LINES;

//Initialize the LINES structure
void lines_init(LINES *l, int fd){
    l->buf = malloc(BUFSIZE);
    l->pos = 0;
    l->bytes = 0;
    l->fd = fd;
}

//Destroy the LINES structure and free allocated memory
void lines_destroy(LINES *l){
    free(l->buf);
}

//Read the next line from the file descriptor
char *lines_next(LINES *l){
    char *line = NULL;
    int linelen = 0;

    if(l->bytes < 0) return NULL;

    do{
        int segstart = l->pos;
        while(l->pos < l->bytes){
            if(l->buf[l->pos] == '\n'){
                int seglen = l->pos - segstart;
                line = realloc(line, linelen + seglen + 1);
                memcpy(line + linelen, l->buf + segstart, seglen);
                line[linelen + seglen] = '\0';
                l->pos++;
                
                return line;
            }

            l->pos++;
        }

        if (segstart < l->pos) {
            int seglen = l->pos - segstart;
            line = realloc(line, linelen + seglen + 1);
            memcpy(line + linelen, l->buf + segstart, seglen);
            linelen = linelen + seglen;
            line[linelen] = '\0';
        }

        l->pos = 0;
        l->bytes = read(l->fd, l->buf, BUFSIZE);
    }while (l->bytes > 0);

    if (linelen > 0){
        line[linelen] = '\0';
        l->bytes = -1;
        return line;
    }
    
    l->bytes = -1;

    return line;
}

int main(int argc, char *argv[]) {
    if(argc == 1){
        interactive_mode = 1; // Non-interactive mode
    }
    
    else {
        interactive_mode = 0; // Interactive mode
        int fd = open(argv[1], O_RDONLY);
        if(fd < 0){
            perror("open");
            exit(EXIT_FAILURE);
        }

        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    signal(SIGINT, signal_handler);

    LINES line_reader;
    lines_init(&line_reader, STDIN_FILENO);

    char *line;
    while ((line = lines_next(&line_reader)) != NULL) {
        if (interactive_mode) {
            printf("%s", PROMPT);
            fflush(stdout);
        }

        Job job;
        memset(&job, 0, sizeof(Job));
        parse_line(line, &job);

        execute_job(&job, 0);

        free(line);
        free_job(&job);
    }

    lines_destroy(&line_reader);

    return 0;
}
