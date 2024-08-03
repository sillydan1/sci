#include "cli.h"
#include "log.h"
#include "notify.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

void on_event(struct inotify_event* const e) {
    const char* msg =
        "got an event:\n"
        "  wd: %d\n"
        "  mask: %d\n"
        "  cookie: %d\n"
        "  len: %d\n"
        "  name: %s"
        ;
    log_info(msg, e->wd, e->mask, e->cookie, e->len, e->name);
}

void* listen_for_changes_thread(void* data) {
    const char* f = (const char*)data;
    listen_for_changes(f, &on_event);
    return NULL;
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

    if(!args.file.has_value) {
        fprintf(stderr, "no file provided see -h for usage\n");
        exit(EXIT_FAILURE);
    }
    if(!args.file2.has_value) {
        fprintf(stderr, "no second file provided see -h for usage\n");
        exit(EXIT_FAILURE);
    }

    if(access(args.file.value, F_OK) != 0) {
        fprintf(stderr, "no such file or directory %s\n", args.file.value);
        exit(EXIT_FAILURE);
    }
    if(access(args.file2.value, F_OK) != 0) {
        fprintf(stderr, "no such file or directory %s\n", args.file2.value);
        exit(EXIT_FAILURE);
    }


    log_trace("spawning trigger thread for %s", args.file.value);
    pthread_t file1_thread;
    pthread_attr_t attr1;
    ASSERT_SYSCALL_SUCCESS(pthread_attr_init(&attr1));
    ASSERT_SYSCALL_SUCCESS(pthread_create(&file1_thread, &attr1, &listen_for_changes_thread, (void*)args.file.value));
    ASSERT_SYSCALL_SUCCESS(pthread_attr_destroy(&attr1));

    log_trace("spawning trigger thread for %s", args.file2.value);
    pthread_t file2_thread;
    pthread_attr_t attr2;
    ASSERT_SYSCALL_SUCCESS(pthread_attr_init(&attr2));
    ASSERT_SYSCALL_SUCCESS(pthread_create(&file2_thread, &attr1, &listen_for_changes_thread, (void*)args.file2.value));
    ASSERT_SYSCALL_SUCCESS(pthread_attr_destroy(&attr2));

    pthread_join(file1_thread, NULL);
    pthread_join(file2_thread, NULL);

    free_options(args);
}
