#include "cli.h"
#include "log.h"
#include "notify.h"
#include "pipeline.h"
#include "threadpool.h"
#include "util.h"
#include <stdlib.h>
#include <sys/stat.h>

void executor(void* data) {
    const char* command = data;
    system(command);
}

threadpool* pool = NULL;

void on_event(pipeline_event* const e) {
    if(!threadpool_add_work(pool, executor, (void*)e->command))
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

    struct stat st = {0};
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
