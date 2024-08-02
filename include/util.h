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

#endif
