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
#include "cli.h"
#include "log.h"
#include "notify.h"
#include "pipeline.h"
#include "threadpool.h"
#include "util.h"
#include <fcntl.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <uuid/uuid.h>
#include <wait.h>

threadpool* pool = NULL;
char* log_dir = "./"; // NOTE: must end with a /

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
    if(!fd.has_value)
        return;

    // spawn the process
    pid_t pid;
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, fd.value, STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, fd.value, STDERR_FILENO);
    const pipeline_event* const e = data;
    char* name = join("SCI_PIPELINE_NAME=", e->name);
    char* url = join("SCI_PIPELINE_URL=", e->url);
    char* trigger = join("SCI_PIPELINE_TRIGGER=", e->trigger);
    char* id = join("SCI_PIPELINE_ID=", pipeline_id);
    char* envp[] = { name, url, trigger, id, NULL };
    char* argv[] = { "/bin/sh", "-c", e->command, NULL };
    if(posix_spawn(&pid, "/bin/sh", &actions, NULL, argv, envp) != 0) {
        perror("posix_spawn");
        goto end; // I know. The raptors have picked up the scent. I'll just have to mask it with more poopy code.
    }
    log_trace("{%s} (%s) spawned", pipeline_id, e->name);

    // Wait for process to complete
    int status;
    waitpid(pid, &status, 0);
    if(WIFEXITED(status))
        log_trace("{%s} (%s) exited with status %d", pipeline_id, e->name, WEXITSTATUS(status));
end:
    close(fd.value);
    free(pipeline_id);
    free(name);
    free(url);
    free(trigger);
    free(id);
}

void on_event(pipeline_event* const e) {
    if(!threadpool_add_work(pool, executor, (void*)e))
        log_error("could not add work to the threadpool");
}

void* listen_for_changes_thread(void* data) {
    const pipeline_conf* conf = (const pipeline_conf*)data;
    while(1) // TODO: Should be while(sigint_has_not_been_caught) instead
        listen_for_changes(conf, &on_event);
    // We're now done with the config.
    free(conf->name);
    free(conf->url);
    free(conf->trigger);
    free(conf->command);
    free(data);
    return NULL;
}

// Spawn a thread that fires events whenever the provided trigger file has changed.
// Usage:
// `pthread_t t = spawn_listener(config);`
// `pthread_join(t, NULL);`
pthread_t spawn_listener(const pipeline_conf* conf) {
    log_info("spawning trigger thread for %s", conf->name);
    pthread_t result;
    ASSERT_SYSCALL_SUCCESS(pthread_create(&result, NULL, &listen_for_changes_thread, (void*)conf));
    pthread_setname_np(result, "sci-listener");
    return result;
}

void config_interpret_line(const char* line) {
    optional_pipeline_conf conf = pipeline_create(line);
    if(!conf.has_value) {
        log_error("unable to register pipeline");
        return;
    }
    char* dest;
    // NOTE: trigger names are allowed max 32 characters
    dest = malloc(sizeof(*dest) * (9+33));
    dest[0] = '\0';
    strncat(dest, "/tmp/sci/", 10);
    strncat(dest, conf.value->trigger, 33);
    free(conf.value->trigger);
    conf.value->trigger = dest;
    pthread_t t = spawn_listener(conf.value);
    pipeline_register(t);
}

int main(int argc, char** argv) {
    cli_options args = parse(argc, argv);
    log_settings settings;
    settings.level = args.verbosity;
    settings.use_colors = args.use_colors;
    settings.out_file = args.log_file.has_value ? fopen(args.log_file.value, "w+") : stdout;
    log_init(settings);

    if(args.help) {
        print_help(stdout, argv[0]);
        exit(EXIT_SUCCESS);
    }

    if(args.version) {
        fprintf(stdout, SCI_VERSION "\n");
        exit(EXIT_SUCCESS);
    }

    if(!args.config_file.has_value) {
        fprintf(stderr, "no pipeline config file provided see -h for usage\n");
        exit(EXIT_FAILURE);
    }

    if(args.pipeline_log_dir.has_value)
        log_dir = args.pipeline_log_dir.value;

    struct stat st = {0};
    if(stat(log_dir, &st) == -1)
        mkdir(log_dir, 0700);

    if(stat("/tmp/sci", &st) == -1)
        mkdir("/tmp/sci", 0700);

    if(access(args.config_file.value, F_OK) != 0) {
        fprintf(stderr, "no such file or directory %s\n", args.config_file.value);
        exit(EXIT_FAILURE);
    }
    
    pool = threadpool_create(args.executors);
    per_line(args.config_file.value, &config_interpret_line);

    pipeline_loop();
    threadpool_destroy(pool);
}
