#define _POSIX_C_SOURCE 200809L
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

int shell_signal_exit = 0; // Flag to indicate shell should exit
int shell_exit_status = EXIT_SUCCESS; // Exit status to return when shell exits

void signal_handler(int signo) {        
    (void)signo; // Unused parameter

    if (interactive_mode) {
        write(STDOUT_FILENO, "\n", 1);
        //write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
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

/*int main(int argc, char *argv[]) {
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

    signal(SIGINT, signal_handler); */

    int main(int argc, char *argv[]) {
    if (argc > 2) {
         fprintf(stderr, "Usage: %s [batchfile]\n", argv[0]); return EXIT_FAILURE; 
        }
    if (argc == 2) {
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0) { perror(argv[1]); return EXIT_FAILURE; }
        if (dup2(fd, STDIN_FILENO) < 0) {
             perror("dup2"); 
             close(fd); 
             return EXIT_FAILURE; 
            }
        close(fd);
    }

    interactive_mode = isatty(STDIN_FILENO);


    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
         perror("sigaction"); return EXIT_FAILURE; 
        }

    LINES line_reader;
    lines_init(&line_reader, STDIN_FILENO);

    char *line;

    if(interactive_mode) {
        printf("Welcome to my shell! \n");
    }
    while (1) {
        if (interactive_mode) {
            printf("%s", PROMPT);
            fflush(stdout);
        }

        line = lines_next(&line_reader);
        if (line == NULL)
            break;

        Job job;
        memset(&job, 0, sizeof(Job));
        parse_line(line, &job);

        execute_job(&job, 0);

        if (shell_signal_exit) {
            free(line);
            free_job(&job);
            break;
        }

        free(line);
        free_job(&job);
    }

    lines_destroy(&line_reader);

    if (interactive_mode){
        printf("Mysh: exiting\n");
        fflush(stdout);
    }

    return shell_exit_status;
}

