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
#ifndef SCI_UTIL_H
#define SCI_UTIL_H
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#define ASSERT_SYSCALL_SUCCESS(fd) \
    do { \
        if ((fd) == -1) { \
            fprintf(stderr, "Assertion failed: %s, errno: %d, error: %s\n", #fd, errno, strerror(errno)); \
            assert(fd != -1); \
        } \
    } while (0)
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))

// remove whitespace characters on both ends of the string.
// Retuirns a new string that you must free.
char* trim(const char* const str);

typedef void(*line_handler)(const char*);
void per_line(const char* file, line_handler handler);

char* join(const char* a, const char* b);
char* join3(const char* a, const char* b, const char* c);
char* join4(const char* a, const char* b, const char* c, const char* d);
char* join5(const char* a, const char* b, const char* c, const char* d, const char* e);
char* join6(const char* a, const char* b, const char* c, const char* d, const char* e, const char* f);

const char* skip_arg(const char* cp);
char* skip_spaces(const char* str);
int count_argc(const char* str);
char** argv_split(const char* str, int* argc_out);
void argv_free(char** argv);
int which(const char* program_name, char* out_full_program, int max_path);

#endif
