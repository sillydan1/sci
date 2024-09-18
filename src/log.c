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
#include "log.h"
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
log_settings g_log_settings;
bool g_log_initialized = false;

void log_init(log_settings settings) {
    g_log_settings.level = settings.level;
    g_log_settings.use_colors = settings.use_colors;
    g_log_settings.out_file = settings.out_file;
    g_log_initialized = true;
}

#define COLOR_TRACE "\x1b[94m"
#define COLOR_INFO "\x1b[32m"
#define COLOR_WARN "\x1b[33m"
#define COLOR_ERROR "\x1b[31m"
#define COLOR_RESET "\x1b[0m"
#define COLOR_FILE "\x1b[90m"

const char* get_level_color(int level) {
    switch(level) {
        case LOG_TRACE:
            return COLOR_TRACE;
        case LOG_INFO:
            return COLOR_INFO;
        case LOG_WARN:
            return COLOR_WARN;
        case LOG_ERROR:
            return COLOR_ERROR;
        default:
            return COLOR_INFO;
    }
}

const char* get_level_name(int level) {
    switch(level) {
        case LOG_TRACE:
            return "TRACE";
        case LOG_INFO:
            return "INFO";
        case LOG_WARN:
            return "WARN";
        case LOG_ERROR:
            return "ERROR";
        default:
            return "LOG";
    }
}

struct tm* g_log_tm = NULL;
void log_log(const char* file, int line, int level, const char* fmt, ...) {
    if(level > g_log_settings.level)
        return;
    char timestamp[16];
    if(g_log_tm == NULL) {
        time_t t = time(NULL);
        g_log_tm = localtime(&t);
    }
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", g_log_tm);
    g_log_tm = NULL;

    const char* level_color = get_level_color(level);
    const char* level_name = get_level_name(level);

    pthread_mutex_lock(&log_mutex);
    if(g_log_settings.use_colors)
        fprintf(g_log_settings.out_file, "%s %s%-5s"COLOR_RESET" "COLOR_FILE"%s:%d:"COLOR_RESET" ", timestamp, 
                level_color, level_name, file, line);
    else
        fprintf(g_log_settings.out_file, "%s %-5s %s:%d: ", timestamp, level_name, file, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(g_log_settings.out_file, fmt, args);
    va_end(args);
    fprintf(g_log_settings.out_file, "\n");
    fflush(g_log_settings.out_file);
    pthread_mutex_unlock(&log_mutex);
}
