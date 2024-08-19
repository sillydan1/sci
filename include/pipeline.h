/**
 * sci - a simple ci system
   Copyright (C) 2024 Asger Gitz-Johansen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SCI_PIPELINE_H
#define SCI_PIPELINE_H
#include "optional.h"
#include <pthread.h>

typedef struct {
    char* name;
    char* url;
    char* trigger;
    char* command;
} pipeline_conf;
typedef optional_type(pipeline_conf*) optional_pipeline_conf;

typedef struct {
    const struct inotify_event* event;
    char* name;
    char* url;
    char* trigger;
    char* command;
} pipeline_event;

// create a new pipeline_conf struct instance based on a configuration line.
optional_pipeline_conf pipeline_create(const char* config_line);
void pipeline_register(pthread_t thread);
void pipeline_loop();

#endif
