#include "cli.h"
#include "notify.h"
#include <stdlib.h>
#include <unistd.h>

void on_notify_event(struct inotify_event* const e) {
    fprintf(stdout, "got an event:\n");
    fprintf(stdout, "  wd: %d\n", e->wd);
    fprintf(stdout, "  mask: %d\n", e->mask);
    fprintf(stdout, "  cookie: %d\n", e->cookie);
    fprintf(stdout, "  len: %d\n", e->len);
    fprintf(stdout, "  name: %s\n", e->name);
}

int main(int argc, char** argv) {
    struct cli_options args = parse(argc, argv);

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
        listen_for_changes(args.file.value, &on_notify_event);
    }

    free_options(args);
}
