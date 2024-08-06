#include "notify.h"
#include "util.h"
#include "log.h"

#define EV_SIZE sizeof(struct inotify_event)
#define BUF_LEN EV_SIZE * 32

void listen_for_changes(const pipeline_conf* config, notify_callback callback) {
    // TODO: callback is a bit slow sometimes. We should also poll once after calling callback
    const char* filename = config->trigger;
    if(access(filename, F_OK) != 0) {
        log_trace("file does not exist yet, creating it.");
        FILE* f = fopen(filename, "w+");
        fclose(f);
    }
    int fd = inotify_init();
    ASSERT_SYSCALL_SUCCESS(fd);
    inotify_add_watch(fd, filename, IN_ATTRIB);
    log_trace("listening for changes in file: %s", filename);
    char buffer[BUF_LEN];
    int r = read(fd, buffer, BUF_LEN);
    assert(r != -1);
    for(int i = 0; i < r; ) {
        struct inotify_event* e = (struct inotify_event*)&buffer[i];
        pipeline_event ev;
        ev.event = e;
        ev.name = config->name;
        ev.url = config->url;
        ev.trigger = config->trigger;
        ev.command = config->command;
        callback(&ev);
        i += EV_SIZE + e->len;
    }
    ASSERT_SYSCALL_SUCCESS(close(fd));
}
