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
// Even root itself will be free'd by this function so it should be discarded as well.
// This function is not thread-safe.
void clear_thread_list(pthread_list_node* root);

#endif
