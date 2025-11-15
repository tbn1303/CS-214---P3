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

    char *path_env = getenv("PATH");
    if (!path_env) return NULL;

    char *path_copy = strdup(path_env);
    char *token = strtok(path_copy, ":");
    while (token) {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", token, command);
        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }
        token = strtok(NULL, ":");
    }
    free(path_copy);
    return NULL;
}