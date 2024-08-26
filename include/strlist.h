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
#ifndef SCI_STRLIST_H
#define SCI_STRLIST_H

// doubly linked list
typedef struct strlist_node {
    char* str;
    struct strlist_node* previous;
    struct strlist_node* next;
} strlist_node;

// Create a new root node.
// This function is not threadsafe.
strlist_node* create_strlist_node(char* str);

// Add a new string to the string list.
// This function is not threadsafe.
strlist_node* add_str(char* str, strlist_node* root);

// add a new string list node to the list.
// This function is not threadsafe.
strlist_node* add_str_node(strlist_node* str_node, strlist_node* root);

// Remove a string list node from the list.
// This will free the str and stitch the "previous" and "next" ptrs.
// This function is not threadsafe.
void remove_strlist_node(strlist_node* node);

// Completely clear the list.
// The list is completely invalid after this call and should be discarded.
// root itself will not be free'd by this function, but all content will be.
// This function is not threadsafe.
void clear_strlist(strlist_node* root);

// Convert a strlist to an array of strings.
// Note that this copies the strlist, you still have to free it.
// The array itself is NULL terminated, enabling you to iterate to the end.
// The array should be free'd, as well as each of the entries.
// Returns NULL if the provided strlist is empty.
char** strlist_to_array(strlist_node* root);

#endif
