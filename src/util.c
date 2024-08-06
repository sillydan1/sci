#include "util.h"
#include "log.h"
#include <ctype.h>
#include <stdlib.h>

char* trim(const char* const str) {
    char* begin = strdup(str);
    char* end;
    while(isspace((unsigned char)*begin))
        begin++;
    if(*begin == 0)
        return begin;
    end = begin + strlen(begin) - 1;
    while(end > begin && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';
    return begin;
}

void per_line(const char* file, line_handler handler) {
    FILE* stream;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    log_trace("reading file %s", file);
    stream = fopen(file, "r");
    if(stream == NULL) {
        perror("fopen");
        return;
    }
    while((nread = getline(&line, &len, stream)) != -1) {
        char* line_trimmed = trim(line);
        handler(line_trimmed);
        free(line_trimmed);
    }
    free(line);
    fclose(stream);
}

char* join(const char* a, const char* b) {
    size_t alen = strlen(a);
    size_t blen = strlen(b);
    char* result = malloc(alen + blen + 1);
    sprintf(result, "%s%s", a, b);
    return result;
}
