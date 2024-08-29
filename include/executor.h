#ifndef SCI_EXECUTOR_H
#define SCI_EXECUTOR_H
#include "strlist.h"

void executor(void* pipeline_event);
void set_logdir(const char* logdir);
void set_working_directory(const char* cwd);
void set_shared_environment(const strlist_node* root);

#endif
