#ifndef SCI_API_H
#define SCI_API_H

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
