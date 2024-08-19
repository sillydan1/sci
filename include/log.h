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
#ifndef SCI_LOG_H
#define SCI_LOG_H
#include <stdio.h>
#include <stdbool.h>

enum {
    LOG_TRACE = 4,
    LOG_INFO = 3,
    LOG_WARN = 2,
    LOG_ERROR = 1,
    LOG_NOTHING = 0
};
typedef struct {
    int level;
    bool use_colors;
    FILE* out_file;
} log_settings;

void log_log(const char* file, int line, int level, const char* fmt, ...);
void log_init(log_settings settings);

#define log_trace(...) log_log(__FILE__, __LINE__, LOG_TRACE, __VA_ARGS__)
#define log_info(...)  log_log( __FILE__, __LINE__, LOG_INFO, __VA_ARGS__)
#define log_warn(...)  log_log( __FILE__, __LINE__, LOG_WARN, __VA_ARGS__)
#define log_error(...) log_log(__FILE__, __LINE__, LOG_ERROR, __VA_ARGS__)

#endif
