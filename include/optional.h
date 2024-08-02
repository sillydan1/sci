#ifndef SCI_OPTIONAL_H
#define SCI_OPTIONAL_H
#include <stdbool.h>

#define optional_type(type) struct { bool has_value; type value; }
typedef optional_type(int) optional_int;
typedef optional_type(float) optional_float;
typedef optional_type(char*) optional_str;
typedef optional_type(const char*) optional_cstr;

#endif
