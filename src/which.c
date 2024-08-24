/**
 * sci - a simple ci system
   Copyright (C) 2024 Asger Gitz-Johansen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "log.h"
#include <assert.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int which(const char* program_name, char* out_full_program, int max_path) {
    assert(out_full_program);
    assert(max_path > 0);
    // sanity check - maybe program_name is actually a full-path to begin with
    if(access(program_name, X_OK) == 0) {
        snprintf(out_full_program, max_path, "%s", program_name);
        return 0;
    }
    char* path = getenv("PATH");
    if (path == NULL) {
        log_error("PATH environment variable not found.");
        return -1;
    }
    char* path_cpy = strdup(path);
    char* dir = strtok(path_cpy, ":");
    char full_path[PATH_MAX];
    while(dir != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, program_name);
        if(access(full_path, X_OK) == 0) {
            snprintf(out_full_program, max_path, "%s", full_path);
            free(path_cpy);
            return 0;
        }
        dir = strtok(NULL, ":");
    }
    log_error("'%s' not found in PATH", program_name);
    free(path_cpy);
    return -1;
}
