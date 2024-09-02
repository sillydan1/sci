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
#ifndef SCI_NOTIFY_H
#define SCI_NOTIFY_H
#include "pipeline.h"
#include <sys/inotify.h>

typedef void(*notify_callback)(pipeline_event* const);
typedef void(*config_change_callback)();

// Start listening for changes to the provided file.
// Note that the `struct inotify_event*` provided is a managed pointer.
void listen_for_changes(const pipeline_conf* config, notify_callback callback);

void listen_for_config_changes(const char* config_filepath, config_change_callback callback);

#endif
