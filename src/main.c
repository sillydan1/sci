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

void executor(void* data) {
    const pipeline_event* const e = data;
    pid_t pid;
    uuid_t uuid;
    uuid_generate(uuid);
    char* pipeline_id = malloc(32);
    uuid_unparse_lower(uuid, pipeline_id);
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    char* log_file = join(pipeline_id, ".log");
    char* log_filepath = join(log_dir, log_file);
    int fd = open(log_filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return;
    }
    posix_spawn_file_actions_adddup2(&actions, fd, STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, fd, STDERR_FILENO);
    char* name = join("SCI_PIPELINE_NAME=", e->name);
    char* url = join("SCI_PIPELINE_URL=", e->url);
    char* trigger = join("SCI_PIPELINE_TRIGGER=", e->trigger);
    char* id = join("SCI_PIPELINE_ID=", pipeline_id);
    char *envp[] = {
        name,
        url,
        trigger,
        id,
        NULL
    };
    char* argv[] = { "/bin/sh", "-c", e->command, NULL };
    if(posix_spawn(&pid, "/bin/sh", &actions, NULL, argv, envp) != 0) {
        perror("posix_spawn");
        return;
    }
    log_trace("{%s} spawned (%s)", pipeline_id, e->name);
    int status;
    waitpid(pid, &status, 0);
    if(WIFEXITED(status))
        log_trace("{%s} exited with status %d", pipeline_id, WEXITSTATUS(status));
    // TODO: clean this function up!
    free(pipeline_id);
    free(log_file);
    free(log_filepath);
    free(name);
    free(url);
    free(trigger);
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
        fprintf(stderr, "no file provided see -h for usage\n");
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

    // BOOKMARK: You were reading :Man system.unit and :Man systemd.service as preperation on making a systemd unit file
    // This will be needed for the .deb package, as well as the arch linux package.
    // alpine linux is using OpenRC (cool), which complicates things a little bit, but shouldn't be too bad. The wiki is
    // generally really well written. Otherwise, I am sure that both wiki.gentoo and wiki.archlinux have great pages too
    // docker is super easy, just make a dockerfile - only concern is the trigger files.
    pipeline_loop();
    threadpool_destroy(pool);
}
