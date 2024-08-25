#include "strlist.h"
#include <stdlib.h>
#include <string.h>

#define MAX_STRLEN 512

strlist_node* create_strlist_node(char* str) {
    strlist_node* new = malloc(sizeof(strlist_node));
    new->previous = NULL;
    new->next = NULL;
    if(str)
        new->str = strndup(str, MAX_STRLEN);
    else
        new->str = NULL;
    return new;
}

strlist_node* add_str(char* str, strlist_node* root) {
    strlist_node* cursor = root;
    while(cursor->next != NULL)
        cursor = cursor->next;
    strlist_node* new = malloc(sizeof(strlist_node));
    new->previous = cursor;
    new->next = NULL;
    new->str = strndup(str, MAX_STRLEN);
    cursor->next = new;
    return new;
}

strlist_node* add_str_node(strlist_node* root, strlist_node* node) {
    strlist_node* cursor = root;
    while(cursor->next != NULL)
        cursor = cursor->next;
    node->previous = cursor;
    node->next = NULL;
    cursor->next = node;
    return node;
}

void remove_strlist_node(strlist_node* node) {
    strlist_node* prev = node->previous;
    strlist_node* next = node->next;
    if(node->str)
        free(node->str);
    node->str = NULL;
    free(node);
    if(prev != NULL)
        prev->next = next;
    if(next != NULL)
        next->previous = prev;
}

void clear_strlist(strlist_node* root) {
    strlist_node* cursor = root;
    while(cursor != NULL) {
        cursor = cursor->next;
        if(cursor->str)
            free(cursor->str);
        cursor->str = NULL;
        free(cursor->previous);
    }
}
