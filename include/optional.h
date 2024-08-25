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
#ifndef SCI_OPTIONAL_H
#define SCI_OPTIONAL_H
#include "strlist.h"
#include <stdbool.h>

#define optional_type(type) struct { bool has_value; type value; }
typedef optional_type(int) optional_int;
typedef optional_type(float) optional_float;
typedef optional_type(char*) optional_str;
typedef optional_type(const char*) optional_cstr;
typedef optional_type(strlist_node*) optional_strlist;

#endif
