#include "log.h"
#include "pipeline.h"
#include "threadlist.h"
#include "util.h"
#include <regex.h>
#include <stdlib.h>

pthread_list_node* root = NULL;

optional_pipeline_conf pipeline_create(const char* config_line) {
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
        opts[i] = strndup(config_line + off, len);
        cursor += pmatch[0].rm_eo;
    }
    if(i != 4) {
        log_error("invalid configuration!\nline is invalid: \"%s\"");
        for(int j = i-1; j >= 0; j--)
            free(opts[j]);
        return result;
    }

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

void pipeline_register(pthread_t thread) {
    if(root == NULL) {
        root = create_thread_node(thread);
        return;
    }
    add_thread(thread, root);
}

void pipeline_loop() {
    clear_thread_list(root);
    root = NULL;
}
