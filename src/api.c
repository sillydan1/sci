#include "api.h"
#include "log.h"
#include "util.h"
#include <linux/limits.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Make sure to write a manpage sci_api(7) for this api
#define LIST_REQ "list"
#define MQ_MAX_SIZE 8192

bool api_is_running = false;
mqd_t api_out;
mqd_t api_in;
pipeline_info_node* api_list_root = NULL;
pthread_mutex_t info_list_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    attr.mq_msgsize = MQ_MAX_SIZE;
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
        if(mq_receive(api_in, msg, MQ_MAX_SIZE, NULL) == -1) {
            perror("mq_receive");
            return NULL;
        }
        api_handle_request(msg);
        memset(msg, '\0', MQ_MAX_SIZE);
    }
    pthread_mutex_lock(&info_list_mutex);
    clear_info_list(api_list_root);
    pthread_mutex_unlock(&info_list_mutex);
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
    mq_unlink("/sci_tx");
    mq_unlink("/sci_rx");
}

char* get_timestamp() {
    char* timestamp = malloc(sizeof(char) * 16);
    memset(timestamp, '\0', 16);
    time_t t;
    time(&t);
    struct tm* g_log_tm = localtime(&t);
    strftime(timestamp, 16, "%H:%M:%S", g_log_tm);
    return timestamp;
}

void api_list_running_pipelines() {
    char* msg = strdup("list ");
    pthread_mutex_lock(&info_list_mutex);
    pipeline_info_node* cursor = api_list_root;
    char* sep = "";
    while(cursor != NULL) {
        char* _msg = join5(msg, sep, cursor->info.pipeline_id, " ", cursor->info.start_timestamp);
        free(msg);
        msg = _msg;
        cursor = cursor->next;
        sep = "\n";
    }
    pthread_mutex_unlock(&info_list_mutex);
    log_trace("asked for running pipelines:");
    log_trace(msg);
    if(mq_send(api_out, msg, strnlen(msg, 2048), 1) == -1)
        perror("mq_send");
    free(msg);
}

void api_pipeline_started(const char* pipeline_id, const char* name) {
    if(!api_is_running)
        return;

    // Add the pipeline to the list.
    pipeline_info info;
    info.start_timestamp = get_timestamp();
    info.pipeline_id = strdup(pipeline_id);
    pthread_mutex_lock(&info_list_mutex);
    if(info_node_count(api_list_root) <= 0)
        api_list_root = create_info_node(info);
    else
        add_info(api_list_root, info);
    pthread_mutex_unlock(&info_list_mutex);

    // Tell people that the pipeline started.
    char* msg = join4("pipeline_new ", pipeline_id, " ", info.start_timestamp);
    if(mq_send(api_out, msg, strnlen(msg, 256), 1) == -1)
        perror("mq_send");
    free(msg);
}

void api_pipeline_ended(const char* pipeline_id, const char* name, int exit_code) {
    if(!api_is_running)
        return;

    // Remove the pipeline from the list.
    pthread_mutex_lock(&info_list_mutex);
    pipeline_info_node* node = find_info_node(pipeline_id, api_list_root);
    if(node != NULL) {
        if(node == api_list_root)
            api_list_root = node->next;
        remove_info_node(node);
    } else
        log_error("could not find pipeline id '%s' in running pipelines", pipeline_id);
    pthread_mutex_unlock(&info_list_mutex);

    // Tell people that the pipeline ended.
    char exit_code_str[64];
    sprintf(exit_code_str, " %d", exit_code);
    char* msg = join5("pipeline_end ", pipeline_id, " ", name, exit_code_str);
    if(mq_send(api_out, msg, strnlen(msg, MQ_MAX_SIZE), 1) == -1)
        perror("mq_send");
    free(msg);
}

void api_started() {
    if(!api_is_running)
        return;
    if(mq_send(api_out, "sci_started", 12, 1) == -1)
        perror("mq_send");
}

pipeline_info_node* create_info_node(pipeline_info info) {
    pipeline_info_node* new = malloc(sizeof(pipeline_info_node));
    new->info = info;
    new->next = NULL;
    new->previous = NULL;
    return new;
}

pipeline_info_node* add_info(pipeline_info_node* root, pipeline_info info) {
    pipeline_info_node* cursor = root;
    while(cursor->next != NULL)
        cursor = cursor->next;
    pipeline_info_node* new = malloc(sizeof(pipeline_info_node));
    new->previous = cursor;
    new->next = NULL;
    new->info = info;
    cursor->next = new;
    return new;
}

pipeline_info_node* add_info_node(pipeline_info_node* root, pipeline_info_node* node) {
    pipeline_info_node* cursor = root;
    while(cursor->next != NULL)
        cursor = cursor->next;
    node->previous = cursor;
    node->next = NULL;
    cursor->next = node;
    return node;
}

void remove_info_node(pipeline_info_node* node) {
    pipeline_info_node* prev = node->previous;
    pipeline_info_node* next = node->next;
    free(node->info.pipeline_id);
    free(node->info.start_timestamp);
    free(node);
    if(prev != NULL)
        prev->next = next;
    if(next != NULL)
        next->previous = prev;
}

void clear_info_list(pipeline_info_node* root) {
    pipeline_info_node* cursor = root;
    while(cursor != NULL) {
        pipeline_info_node* prev = cursor;
        cursor = cursor->next;
        free(prev->info.pipeline_id);
        free(prev->info.start_timestamp);
        free(prev);
    }
}

pipeline_info_node* find_info_node(const char* info_id, pipeline_info_node* root) {
    pipeline_info_node* cursor = root;
    while(cursor != NULL) {
        if(strncmp(info_id, cursor->info.pipeline_id, 256) == 0)
            return cursor;
        cursor = cursor->next;
    }
    return NULL;
}

size_t info_node_count(pipeline_info_node* root) {
    size_t result = 0;
    pipeline_info_node* cursor = root;
    while(cursor != NULL) {
        cursor = cursor->next;
        result++;
    }
    return result;
}
