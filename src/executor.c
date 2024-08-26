#include "executor.h"
#include "log.h"
#include "optional.h"
#include "pipeline.h"
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

void set_logdir(const char* logdir) {
    log_dir = logdir;
    struct stat st = {0};
    if(stat(log_dir, &st) == -1)
        mkdir(log_dir, 0700);
}

char* create_pipeline_id() {
    uuid_t uuid;
    uuid_generate(uuid);
    char* pipeline_id = malloc(32);
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
    char* path = join("PATH=", getenv("PATH"));
    char* name = join("SCI_PIPELINE_NAME=", e->name);
    char* url = join("SCI_PIPELINE_URL=", e->url);
    char* trigger = join("SCI_PIPELINE_TRIGGER=", e->trigger);
    char* id = join("SCI_PIPELINE_ID=", pipeline_id);
    char* envp[] = { path, name, url, trigger, id, NULL };
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
    free(name);
    free(url);
    free(trigger);
    free(id);
}
