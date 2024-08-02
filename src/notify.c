#include "notify.h"
#include "util.h"

#define EV_SIZE sizeof(struct inotify_event)
#define BUF_LEN EV_SIZE * 32

void listen_for_changes(const char* filename, notify_callback callback) {
    int fd = inotify_init();
    ASSERT_SYSCALL_SUCCESS(fd);
    inotify_add_watch(fd, filename, IN_ATTRIB);

    fprintf(stdout, "listening for changes in file: %s\n", filename);

    char buffer[BUF_LEN];
    int r = read(fd, buffer, BUF_LEN);
    assert(r != -1);
    for(int i = 0; i < r; ) {
        struct inotify_event* e = (struct inotify_event*)&buffer[i];
        callback(e);
        i += EV_SIZE + e->len;
    }
    ASSERT_SYSCALL_SUCCESS(close(fd)); // TODO: have a hashmap of threads (see readme)
}
