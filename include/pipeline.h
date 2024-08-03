#ifndef SCI_PIPELINE_H
#define SCI_PIPELINE_H
#include "optional.h"
#include <pthread.h>
#include <sys/inotify.h>

typedef struct {
    char* name;
    char* url;
    char* trigger;
    char* command;
} pipeline_conf;
typedef optional_type(pipeline_conf*) optional_pipeline_conf;

typedef struct {
    const struct inotify_event* event;
    const char* command;
} pipeline_event;

// create a new pipeline_conf struct instance based on a configuration line.
optional_pipeline_conf pipeline_create(const char* config_line);
void pipeline_register(pthread_t thread);
void pipeline_loop();

#endif
