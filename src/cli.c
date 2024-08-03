#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "cli.h"

cli_options new_options() {
    cli_options result;
    result.file.has_value = false;
    result.file.value = NULL;

    result.file2.has_value = false;
    result.file2.value = NULL;

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
    return result;
}

void free_options(cli_options v) {
    if(v.file.has_value)
        free(v.file.value);
    if(v.file2.has_value)
        free(v.file2.value);
}

//                                                         <max
const char* optstring = "f:F:v:Cl:hV";
const char* help_msg = 
    "Usage: %s [-v level] [-C] [-l file] [-h] [-V]\n"
    "\n"
    SCI_DESCRIPTION "\n"
    "\n"
    "THIS PROGRAM IS STILL JUST A PROTOTYPE, AND NOT\n"
    "ACTUALLY USEFUL YET\n"
    "\n"
    "OPTIONS:\n"
    "  -f file     (WIP) set file\n"
    "  -F file     (WIP) set another file\n"
    "  -v level    Set verbosity level [0-4]\n"
    "  -C          Force color output, ignoring $NO_COLOR\n"
    "  -l file     Set log to output to a file\n"
    "  -h          Show this message and exit\n"
    "  -V          Show version and exit\n"
    ;
//                                                         <max

void print_help(FILE * out, char* prog_name) {
    fprintf(out, help_msg, prog_name);
}

cli_options parse(int argc, char** argv) {
    cli_options options = new_options();
    int opt;
    while((opt = getopt(argc, argv, optstring)) != -1) {
        switch(opt) {
            case 'f':
                options.file.value = strdup(optarg);
                options.file.has_value = true;
                break;
            case 'F':
                options.file2.value = strdup(optarg);
                options.file2.has_value = true;
                break;
            case 'v':
                options.verbosity = atoi(optarg);
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
