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
#include "notify.h"
#include "util.h"
#include "log.h"
#include <stdlib.h>

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
        pipeline_event* ev = malloc(sizeof(pipeline_event));
        ev->event = e;
        ev->name = config->name;
        ev->url = config->url;
        ev->trigger = config->trigger;
        ev->command = config->command;
        callback(ev);
        i += EV_SIZE + e->len;
    }
    ASSERT_SYSCALL_SUCCESS(close(fd));
}
