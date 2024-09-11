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
#include "api.h"
#include "cli.h"
#include "executor.h"
#include "log.h"
#include "notify.h"
#include "pipeline.h"
#include "threadpool.h"
#include "util.h"
#include <mqueue.h>
#include <signal.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

threadpool* worker_pool = NULL;
char* trigger_dir = "/tmp/sci";
bool config_file_changed = false;

void on_event(pipeline_event* const e) {
    if(!threadpool_add_work(worker_pool, executor, (void*)e))
        log_error("could not add work to the threadpool");
}

void listener_thread_cleanup(void* data) {
    // We're now done with the config.
    pipeline_destroy((pipeline_conf*)data);
}

void* listen_for_changes_thread(void* data) {
    pipeline_conf* conf = (pipeline_conf*)data;
    pthread_cleanup_push(listener_thread_cleanup, conf);
    while(1)
        listen_for_changes(conf, &on_event);
    pthread_cleanup_pop(1);
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
    char* new_trigger_val = join3(trigger_dir, "/", conf.value->trigger);
    free(conf.value->trigger);
    conf.value->trigger = new_trigger_val;
    pthread_t t = spawn_listener(conf.value);
    pipeline_register(t);
}

void on_config_file_changed() {
    config_file_changed = true;
    pipeline_cancel();
    log_info("config file changed, reloading...");
}

void* listen_for_config_changes_thread(void* data) {
    while(1)
        listen_for_config_changes((const char*)data, &on_config_file_changed);
    return NULL;
}

void signal_handler(int signal) {
    log_info("signal retrieved");
    if(signal == SIGINT)
        pipeline_cancel();
}

int main(int argc, char** argv) {
    cli_options args = parse(argc, argv);
    log_settings settings;
    settings.level = args.verbosity;
    settings.use_colors = args.use_colors;
    settings.out_file = args.log_file.has_value ? fopen(args.log_file.value, "w+") : stdout;
    log_init(settings);

    signal(SIGINT, signal_handler);

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

    if(access(args.config_file.value, F_OK) != 0) {
        fprintf(stderr, "no such file or directory %s\n", args.config_file.value);
        exit(EXIT_FAILURE);
    }

    if(args.pipeline_log_dir.has_value)
        set_logdir(args.pipeline_log_dir.value);

    if(args.pipeline_cwd.has_value)
        set_working_directory(args.pipeline_cwd.value);

    struct stat st = {0};
    if(stat(trigger_dir, &st) == -1)
        mkdir(trigger_dir, 0700);

    if(args.environment_vars.has_value)
        set_shared_environment(args.environment_vars.value);

    api_start_p();
    
    log_info("spawning trigger thread for config file");
    pthread_t conf_listener;
    ASSERT_SYSCALL_SUCCESS(pthread_create(&conf_listener, NULL, &listen_for_config_changes_thread, (void*)args.config_file.value));
    pthread_setname_np(conf_listener, "sci-conf-listener");
    do {
        config_file_changed = false;
        worker_pool = threadpool_create(args.executors);
        per_line(args.config_file.value, &config_interpret_line);
        log_info("listening for pipeline invocations");
        pipeline_loop();
    } while(config_file_changed);
    pthread_cancel(conf_listener);
    threadpool_destroy(worker_pool);
    destroy_options(args);
    api_destroy();
}
