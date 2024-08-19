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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "cli.h"

cli_options new_options() {
    cli_options result;
    result.config_file.has_value = false;
    result.config_file.value = NULL;
    result.executors = 32;
    result.verbosity = 1;
    result.help = false;
    result.version = false;

    char *no_color = getenv("NO_COLOR");
	bool color = true;
	if(no_color != NULL && no_color[0] != '\0')
		color = false;
    result.use_colors = color;

    result.log_file.has_value = false;
    result.log_file.value = NULL;

    result.pipeline_log_dir.has_value = false;
    result.pipeline_log_dir.value = NULL;
    return result;
}

void destroy_options(cli_options v) {
    if(v.config_file.has_value)
        free(v.config_file.value);
    if(v.log_file.has_value)
        free(v.log_file.value);
    if(v.pipeline_log_dir.has_value)
        free(v.pipeline_log_dir.value);
}

//                                                         <max
const char* optstring = "f:L:e:v:Cl:hV";
const char* help_msg = 
    "%s %s\n"
    "Usage: [-f file] [-L dir] [-e count] [-v level] \n"
    "       [-C] [-l file] [-h] [-V]\n"
    "\n"
    SCI_DESCRIPTION "\n"
    "\n"
    "OPTIONS:\n"
    "  -f file     Set sci config file\n"
    "  -L dir      Set pipeline log output directory\n"
    "  -e count    Set the amount of worker threads\n"
    "  -v level    Set verbosity level [0-4]\n"
    "  -C          Force color output, ignoring $NO_COLOR\n"
    "  -l file     Set sci's log to output to a file\n"
    "  -h          Show this message and exit\n"
    "  -V          Show version and exit\n"
    ;
//                                                         <max

void print_help(FILE * out, char* prog_name) {
    fprintf(out, help_msg, prog_name, SCI_VERSION);
}

cli_options parse(int argc, char** argv) {
    cli_options options = new_options();
    int opt;
    while((opt = getopt(argc, argv, optstring)) != -1) {
        switch(opt) {
            case 'f':
                options.config_file.value = strdup(optarg);
                options.config_file.has_value = true;
                break;
            case 'L':
                options.pipeline_log_dir.value = strdup(optarg);
                options.pipeline_log_dir.has_value = true;
                break;
            case 'v':
                options.verbosity = atoi(optarg);
                break;
            case 'e':
                options.executors = atoi(optarg);
                break;
            case 'C':
                options.use_colors = true;
                break;
            case 'l':
                options.log_file.value = strdup(optarg);
                options.log_file.has_value = true;
                break;
            case 'V':
                options.version = true;
                break;
            case 'h':
                options.help = true;
                break;
            default:
                print_help(stderr, argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    return options;
}
