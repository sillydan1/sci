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

#endif
