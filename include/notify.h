#ifndef SCI_NOTIFY_H
#define SCI_NOTIFY_H
#include <sys/inotify.h>

typedef void(*notify_callback)(struct inotify_event* const);

// Start listening for changes to the provided file.
// Note that the `struct inotify_event*` provided is a managed pointer.
void listen_for_changes(const char* filename, notify_callback callback);

#endif
