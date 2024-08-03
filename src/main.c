#include "cli.h"
#include "log.h"
#include "notify.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

    if(args.file.has_value) {
        if(access(args.file.value, F_OK) != 0) {
            fprintf(stderr, "no such file or directory %s\n", args.file.value);
            exit(EXIT_FAILURE);
        }
        listen_for_changes(args.file.value, &on_event);
    }

    free_options(args);
}
