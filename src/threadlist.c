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
#include "threadlist.h"
#include <stdlib.h>

// TODO: mutex should really be per-list.
pthread_mutex_t threadlist_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_list_node* create_thread_node(pthread_t thread) {
    pthread_list_node* new = malloc(sizeof(pthread_list_node));
    new->previous = NULL;
    new->next = NULL;
    new->thread = thread;
    return new;
}

pthread_list_node* add_thread(pthread_t thread, pthread_list_node* root) {
    pthread_mutex_lock(&threadlist_mutex);
    pthread_list_node* cursor = root;
    while(cursor->next != NULL)
        cursor = cursor->next;
    pthread_list_node* new = malloc(sizeof(pthread_list_node));
    new->previous = cursor;
    new->next = NULL;
    new->thread = thread;
    cursor->next = new;
    pthread_mutex_unlock(&threadlist_mutex);
    return new;
}

pthread_list_node* add_thread_node(pthread_list_node* root, pthread_list_node* node) {
    pthread_mutex_lock(&threadlist_mutex);
    pthread_list_node* cursor = root;
    while(cursor->next != NULL)
        cursor = cursor->next;
    node->previous = cursor;
    node->next = NULL;
    cursor->next = node;
    pthread_mutex_unlock(&threadlist_mutex);
    return node;
}

void remove_thread_node(pthread_list_node* node) {
    pthread_mutex_lock(&threadlist_mutex);
    pthread_list_node* prev = node->previous;
    pthread_list_node* next = node->next;
    free(node);
    if(prev != NULL)
        prev->next = next;
    if(next != NULL)
        next->previous = prev;
    pthread_mutex_unlock(&threadlist_mutex);
}

void clear_thread_list(pthread_list_node* root) {
    pthread_list_node* cursor = root;
    while(cursor != NULL) {
        pthread_join(cursor->thread, NULL);
        cursor = cursor->next;
        free(cursor->previous);
    }
}
