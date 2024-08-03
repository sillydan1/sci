#ifndef SCI_NOTIFY_H
#define SCI_NOTIFY_H
#include "pipeline.h"
#include <sys/inotify.h>

typedef void(*notify_callback)(pipeline_event* const);

// Start listening for changes to the provided file.
// Note that the `struct inotify_event*` provided is a managed pointer.
void listen_for_changes(const pipeline_conf* config, notify_callback callback);

#endif
