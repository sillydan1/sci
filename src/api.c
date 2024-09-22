#include "api.h"
#include "log.h"
#include "util.h"
#include <linux/limits.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Make sure to write a manpage for this api
#define LIST_REQ "list"
#define MQ_MAX_SIZE 8192

bool api_is_running = false;
mqd_t api_out;
mqd_t api_in;

void api_handle_request(const char* request) {
    log_trace("api request: '%s'", request);

    // list
    if(strncmp(request, LIST_REQ, MQ_MAX_SIZE) == 0) {
        api_list_running_pipelines();
        return;
    }

    // else
    log_error("unrecognized api request: '%s'", request);
}

void* api_start(void* data) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 512;
    attr.mq_curmsgs = 0;

    api_out = mq_open("/sci_tx", O_CREAT | O_WRONLY, 0666, &attr);
    if(api_out == -1) {
        perror("mq_open");
        return NULL;
    }

    api_in = mq_open("/sci_rx", O_CREAT | O_RDONLY, 0666, &attr); // TODO: Consider some better mq names
    if(api_in == -1) {
        perror("mq_open");
        return NULL;
    }

    api_started();

    log_info("api listening for requests");
    char msg[MQ_MAX_SIZE];
    api_is_running = true;
    while(api_is_running) {
        memset(msg, '\0', MQ_MAX_SIZE);
        if(mq_receive(api_in, msg, MQ_MAX_SIZE, NULL) == -1) {
            perror("mq_receive");
            return NULL;
        }
        api_handle_request(msg);
    }
    return NULL;
}

void api_start_p() {
    pthread_t conf_listener;
    ASSERT_SYSCALL_SUCCESS(pthread_create(&conf_listener, NULL, &api_start, (void*)NULL));
    pthread_setname_np(conf_listener, "sci-api");
}

void api_destroy() {
    log_trace("closing api");
    api_is_running = false;
    mq_unlink("/sci");
}

void api_list_running_pipelines() {
    // TODO: you need a way of enumerating the pipeline ids before this can be implemented.
    log_error("cannot list running pipelines yet, feature is work-in-progress.");
}

void api_pipeline_started(const char* pipeline_id, const char* name) {
    char* msg = join("pipeline_new ", pipeline_id);
    if(mq_send(api_out, msg, strnlen(msg, 256), 1) == -1)
        perror("mq_send");
    free(msg);
}

void api_pipeline_ended(const char* pipeline_id, const char* name, int exit_code) {
    char exit_code_str[64];
    sprintf(exit_code_str, " %d", exit_code);
    char* msg = join5("pipeline_end ", pipeline_id, " ", name, exit_code_str);
    if(mq_send(api_out, msg, strnlen(msg, 256), 1) == -1)
        perror("mq_send");
    free(msg);
}

void api_started() {
    if(mq_send(api_out, "sci_started", 12, 1) == -1)
        perror("mq_send");
}
