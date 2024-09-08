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
#include "executor.h"
#include "log.h"
#include "optional.h"
#include "pipeline.h"
#include "strlist.h"
#include "util.h"
#include <fcntl.h>
#include <linux/limits.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <uuid/uuid.h>

const char* log_dir = ".";
const char* cwd = "/tmp";
const strlist_node* shared_environment = NULL;

void set_shared_environment(const strlist_node* root) {
    shared_environment = root;
}

void set_logdir(const char* logdir) {
    log_dir = logdir;
    struct stat st = {0};
    if(stat(log_dir, &st) == -1)
        mkdir(log_dir, 0700);
}

void set_working_directory(const char* _cwd) {
    cwd = _cwd;
    struct stat st = {0};
    if(stat(cwd, &st) == -1)
        mkdir(cwd, 0700);
}

char* create_pipeline_id() {
    uuid_t uuid;
    uuid_generate(uuid);
    // example uuid
    // 662ddee9-ee7c-4d13-8999-a2604c6d12d6
    // it's 36 characters (+null)
    char* pipeline_id = malloc(sizeof(char) * 37);
    uuid_unparse_lower(uuid, pipeline_id);
    return pipeline_id;
}

optional_int open_logfile(const char* const pipeline_id) {
    optional_int result;
    result.has_value = false;
    result.value = 0;
    char* log_filepath = join4(log_dir, "/", pipeline_id, ".log");
    int fd = open(log_filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        result.has_value = true;
        result.value = fd;
    } else
        perror("open");
    free(log_filepath);
    return result;
}

void add_joined_str(strlist_node* root, const char* a, const char* b) {
    char* tmp = join(a, b);
    add_str(tmp, root);
    free(tmp);
}

void add_env(strlist_node* root, const char* env) {
    char* tmp = join3(env, "=", getenv(env));
    add_str(tmp, root);
    free(tmp);
}

char** create_environment(const pipeline_event* const e, const char* pipeline_id) {
    char* tmp = join("SCI_PIPELINE_NAME=", e->name);
    strlist_node* env = create_strlist_node(tmp);
    free(tmp);
    add_joined_str(env, "SCI_PIPELINE_URL=", e->url);
    add_joined_str(env, "SCI_PIPELINE_TRIGGER=", e->trigger);
    add_joined_str(env, "SCI_PIPELINE_ID=", pipeline_id);
    if(shared_environment != NULL) {
        const strlist_node* cursor = shared_environment;
        while(cursor != NULL) {
            add_env(env, cursor->str);
            cursor = cursor->next;
        }
    }
    char** envp = strlist_to_array(env);
    clear_strlist(env);
    return envp;
}

void executor(void* data) {
    // Create pipeline id
    char* pipeline_id = create_pipeline_id();

    // Create logfile path
    optional_int fd = open_logfile(pipeline_id);
    if(!fd.has_value) {
        log_error("could not open log file - not starting pipeline");
        return;
    }

    // spawn the process
    pid_t pid;
    const pipeline_event* const e = data;
    char** envp = create_environment(e, pipeline_id);
    int argc;
    char** argv = argv_split(e->command, &argc);
    log_trace("executing pipeline %s with argv:", e->name);
    for(int i = 0; i < argc; i++)
        log_trace("  \"%s\"", argv[i]);
    char arg0[PATH_MAX];
    if(which(argv[0], arg0, PATH_MAX) == -1)
        goto end;

    // fork / cwd / exec idiom
    pid = fork();
    if(pid < 0) {
        perror("fork");
        goto end; // I know. The raptors have picked up the scent. I'll just have to mask it with more stinky code.
    }

    if(pid == 0) {
        // child process
        dup2(fd.value, STDOUT_FILENO);
        dup2(fd.value, STDERR_FILENO);
        char* pipeline_cwd = join3(cwd, "/", pipeline_id);
        struct stat st = {0};
        if(stat(pipeline_cwd, &st) == -1)
            mkdir(pipeline_cwd, 0700);
        chdir(pipeline_cwd);
        free(pipeline_cwd);
        execvpe(arg0, argv, envp);
        return;
    }

    log_info("{%s} (%s) spawned", pipeline_id, e->name);

    // Wait for process to complete
    int status;
    waitpid(pid, &status, 0);
    log_info("{%s} (%s) [pid=%d] exited with status %d", pipeline_id, e->name, pid, status);
    char buf[32];
    sprintf(buf, "exited with status %d", status);
    if(write(fd.value, buf, strnlen(buf, 32)) == -1)
        perror("write");
end:
    argv_free(argv);
    close(fd.value);
    free(pipeline_id);
    pipeline_event_destroy(data);
    char** cursor = envp;
    while(*cursor != NULL) {
        free(*cursor);
        cursor++;
    }
    free(envp);
}
