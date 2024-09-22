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
#include "log.h"
#include "pipeline.h"
#include "threadlist.h"
#include "util.h"
#include <pthread.h>
#include <regex.h>
#include <stdlib.h>

pthread_list_node* pipeline_list_root = NULL;

optional_pipeline_conf pipeline_create(const char* config_line) {
    log_trace("pipeline create");
    optional_pipeline_conf result;
    result.has_value = false;
    const char* pattern = "[^[:blank:]]+|\"[^\"]*\"";
    regex_t reg;
    regmatch_t pmatch[1];
    regoff_t off, len;
    assert(regcomp(&reg, pattern, REG_EXTENDED) == 0);
    const char* cursor = config_line;
    char* opts[4];
    int i = 0;
    for( ; i < 4; i++) {
        if(regexec(&reg, cursor, ARRAY_SIZE(pmatch), pmatch, 0))
            break;
        off = pmatch[0].rm_so + (cursor - config_line);
        len = pmatch[0].rm_eo - pmatch[0].rm_so;
        // Cut off the "-s if it is string-enclosed
        if(config_line[off] == '"' && config_line[off+len-1] == '"')
            opts[i] = strndup(config_line + off+1, len-2);
        else
            opts[i] = strndup(config_line + off, len);
        cursor += pmatch[0].rm_eo;
    }
    if(i != 4) {
        log_error("invalid configuration!");
        log_error("line is invalid: \"%s\"", config_line);
        for(int j = i-1; j >= 0; j--)
            free(opts[j]);
        return result;
    }
    regfree(&reg);

    result.value = malloc(sizeof(pipeline_conf));
    result.value->name = opts[0];
    result.value->url = opts[1];
    result.value->trigger = opts[2];
    result.value->command = opts[3];
    log_trace("read config:");
    log_trace("  name=%s", result.value->name);
    log_trace("  url=%s", result.value->url);
    log_trace("  trigger=%s", result.value->trigger);
    log_trace("  command=%s", result.value->command);
    result.has_value = true;
    return result;
}

void pipeline_destroy(pipeline_conf* conf) {
    log_trace("pipeline destroy");
    free(conf->name);
    free(conf->url);
    free(conf->trigger);
    free(conf->command);
    free(conf);
}

void pipeline_register(pthread_t thread) {
    log_trace("pipeline register thread");
    if(pipeline_list_root == NULL) {
        pipeline_list_root = create_thread_node(thread);
        return;
    }
    add_thread(thread, pipeline_list_root);
}

void pipeline_loop() {
    log_trace("pipeline loop");
    clear_thread_list(pipeline_list_root);
    pipeline_list_root = NULL;
}

void pipeline_cancel() {
    log_trace("cancelling pipeline");
    pthread_list_node* cursor = pipeline_list_root;
    while(cursor != NULL) {
        pthread_cancel(cursor->thread);
        cursor = cursor->next;
    }
}

void pipeline_event_destroy(pipeline_event* ev) {
    log_trace("pipeline event destroy");
    free(ev->name);
    free(ev->trigger);
    free(ev->url);
    free(ev->command);
    free(ev);
}

int pipeline_count() {
    int result = 0;
    pthread_list_node* cursor = pipeline_list_root;
    while(cursor != NULL) {
        cursor = cursor->next;
        result++;
    }
    return result;
}
