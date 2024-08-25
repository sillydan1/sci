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
#include "util.h"
#include <ctype.h>
#include <linux/limits.h>
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

const char* skip_arg(const char* cp) {
	while(*cp && !isspace(*cp))
		cp++;
	return cp;
}

char* skip_spaces(const char* str) {
	while(isspace(*str))
		str++;
	return(char*)str;
}

int count_argc(const char* str) {
	int count = 0;
	while(*str) {
		str = skip_spaces(str);
		if(!*str)
			continue;
		count++;
		str = skip_arg(str);
	}
	return count;
}

void argv_free(char** argv) {
	for(char** p = argv; *p; p++) {
		free(*p);
		*p = NULL;
	}
	free(argv);
}

char** argv_split(const char* str, int* argc_out) {
	int argc = count_argc(str);
	char** result = calloc(argc+1, sizeof(*result));
	if(result == NULL)
		return result;
	if(argc_out)
		*argc_out = argc+1;
	char** argvp = result;
	while(*str) {
		str = skip_spaces(str);
		if(!*str)
			continue;
		const char* p = str;
		str = skip_arg(str);
		char* t = strndup(p, str-p);
		if(t == NULL) {
			perror("strndup");
			argv_free(result);
			return NULL;
		}
		*argvp++ = t;
	}
	*argvp = NULL;
	return result;
}

int which(const char* program_name, char* out_full_program, int max_path) {
	assert(out_full_program);
	assert(max_path > 0);
	// sanity check - maybe program_name is actually a full-path to begin with
	if(access(program_name, X_OK) == 0) {
		snprintf(out_full_program, max_path, "%s", program_name);
		return 0;
	}
	char* path = getenv("PATH");
	if (path == NULL) {
		log_error("PATH environment variable not found.");
		return -1;
	}
	char* path_cpy = strdup(path);
	char* dir = strtok(path_cpy, ":");
	char full_path[PATH_MAX];
	while(dir != NULL) {
		snprintf(full_path, sizeof(full_path), "%s/%s", dir, program_name);
		if(access(full_path, X_OK) == 0) {
			snprintf(out_full_program, max_path, "%s", full_path);
			free(path_cpy);
			return 0;
		}
		dir = strtok(NULL, ":");
	}
	log_error("'%s' not found in PATH", program_name);
	free(path_cpy);
	return -1;
}
