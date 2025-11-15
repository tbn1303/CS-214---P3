#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "mysh.h"

static char *executable_path(const char *command) {
    if (strchr(command, '/')) {
        // Command contains a slash, treat as a path
        if (access(command, X_OK) == 0) {
            return strdup(command);
        } else {
            return NULL;
        }
    }

    const char *default_dirs[] = {"/usr/local/bin", "/usr/bin", "/bin", NULL}; // Default directories
    for (int i = 0; default_dirs[i] != NULL; i++) {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "/%s/%s", default_dirs[i], command);
        if (access(full_path, X_OK) == 0) {
            return strdup(full_path);
        }
    }

    return NULL; // Command not found
}

int is_builtin(Command *cmd) {
    if (strcmp(cmd->argv[0], "cd") == 0 || strcmp(cmd->argv[0], "exit") == 0) {
        return 1; // It's a built-in command
    }

    return 0; // Not a built-in command
}

int execute_builtin(Command *cmd) {
    if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argv[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
            return 1;
        }

        if (chdir(cmd->argv[1]) != 0) {
            perror("cd");
            return 1;
        }

        return 0;
    }
    
    else if (strcmp(cmd->argv[0], "exit") == 0) {
        exit(0);
    }

    return 1; // Not a built-in command
}