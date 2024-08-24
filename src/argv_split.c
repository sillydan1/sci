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
#include "argv_split.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* skip_arg(const char* cp) {
	while(*cp && !isspace(*cp))
		cp++;
	return cp;
}

static char* skip_spaces(const char* str) {
	while(isspace(*str))
		str++;
	return(char*)str;
}

static int count_argc(const char* str) {
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

char** create_argv_shell(const char* str, int* argc_out) {
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
// sci-release https://git.gtz.dk/agj/sci/archive/main.tar.gz manual "scripts/wget-and-sci.sh sci"
