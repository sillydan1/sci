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
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <uuid/uuid.h>

const char* log_dir = "./"; // NOTE: must end with a /
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
    char* log_file = join(pipeline_id, ".log");
    char* log_filepath = join(log_dir, log_file);
    int fd = open(log_filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        result.has_value = true;
        result.value = fd;
    } else
        perror("open");
    free(log_file);
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
    char* tmp = join("PATH=", getenv("PATH")); // TODO: consider removing PATH default, since it can be done as -e PATH
    strlist_node* env = create_strlist_node(tmp);
    free(tmp);
    add_joined_str(env, "SCI_PIPELINE_NAME=", e->name);
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
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, fd.value, STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, fd.value, STDERR_FILENO);
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
    if(posix_spawn(&pid, arg0, &actions, NULL, argv, envp) != 0) {
        perror("posix_spawn");
        goto end; // I know. The raptors have picked up the scent. I'll just have to mask it with more stinky code.
    }
    log_info("{%s} (%s) spawned", pipeline_id, e->name);

    // Wait for process to complete
    int status;
    waitpid(pid, &status, 0);
    log_info("{%s} (%s) exited with status %d", pipeline_id, e->name, status);
    char buf[32];
    sprintf(buf, "exited with status %d", status);
    write(fd.value, buf, strnlen(buf, 32));
end:
    argv_free(argv);
    close(fd.value);
    free(pipeline_id);
    free(data);
    char** cursor = envp;
    while(*cursor != NULL) {
        free(*cursor);
        cursor++;
    }
    free(envp);
}
