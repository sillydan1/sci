#ifndef SCI_LOG_H
#define SCI_LOG_H
#include <stdarg.h>
#include <stdio.h>

// TODO: Thread safety!
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
