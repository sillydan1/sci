#ifndef SCI_THREADLIST_H
#define SCI_THREADLIST_H
#include <pthread.h>

// doubly linked list implementation for managing threads
typedef struct pthread_list_node {
    pthread_t thread;
    struct pthread_list_node* previous;
    struct pthread_list_node* next;
} pthread_list_node;

// Create a new root node.
pthread_list_node* create_thread_node(pthread_t thread);

// Add a new node to the end of the list.
// Returns the new node.
pthread_list_node* add_thread(pthread_t thread, pthread_list_node* root);

pthread_list_node* add_thread_node(pthread_list_node* root, pthread_list_node* node);

// Remove a node from the list.
// This will not call pthread_join, so make sure that the thread is joined before calling.
void remove_thread_node(pthread_list_node* node);

// Completely clear the thread list.
// This will call pthread_join on all nodes.
// The list is completely invalid after this call and should be discarded.
// Note:
//  - `root` has already been free'd.
//  - this function is not thread-safe.
void clear_thread_list(pthread_list_node* root);

#endif
