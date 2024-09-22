#ifndef SCI_API_H
#define SCI_API_H
#include <stdio.h>

typedef struct pipeline_info {
    // Approximate time stamp of when the pipeline started.
    char* start_timestamp;
    // The id of the pipeline.
    char* pipeline_id;
} pipeline_info;

typedef struct pipeline_info_node {
    pipeline_info info;
    struct pipeline_info_node* previous;
    struct pipeline_info_node* next;
} pipeline_info_node;

// Create a new root node.
pipeline_info_node* create_info_node(pipeline_info info);

// Add a new node to the end of the list.
// Returns the new node
pipeline_info_node* add_info(pipeline_info_node* root, pipeline_info info);

// Add a new node to the end of the list.
// Returns the new node
pipeline_info_node* add_info_node(pipeline_info_node* root, pipeline_info_node* node);

// Remove a node from the list.
// Note that this will call free on the provided node.
void remove_info_node(pipeline_info_node* node);

// Clear the list.
// The list is completely invalid after this call and should be discarded.
// Even root itself will be free'd by this function so it should be discarded as well.
void clear_info_list(pipeline_info_node* root);

// Find a node in the list.
// Returns NULL and logs a warning if it could not be found.
pipeline_info_node* find_info_node(const char* info_id, pipeline_info_node* root);

// Get the amount of entries in the list.
// If provided root is NULL, this will just return 0.
size_t info_node_count(pipeline_info_node* root);

// Start the api. This will also trigger an "sci started" event.
// Note that this is a blocking call.
void* api_start(void*);

// Fork the process and have the child run the api.
void api_start_p();

// Destroy all listeners and release the message queue.
void api_destroy();

// Post a newline-separated string with pipeline_id entries
// of the currently running pipelines on the message queue.
void api_list_running_pipelines();

// Trigger a pipeline started event.
void api_pipeline_started(const char* pipeline_id, const char* name);

// Trigger a pipeline ended event.
void api_pipeline_ended(const char* pipeline_id, const char* name, int exit_code);

// Trigger an api started event.
void api_started();

#endif // !SCI_API_H
