#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "cli.h"

struct cli_options new_options() {
    struct cli_options result;
    result.file.has_value = false;
    result.verbosity = 1;
    result.help = false;
    result.version = false;
    return result;
}

void free_options(struct cli_options v) {
    if(v.file.has_value)
        free(v.file.value);
}

//                                                         <max
const char* optstring = "f:v:hV";
const char* help_msg = 
    "Usage: %s [-v level] [-h] [-V]\n"
    "\n"
    SCI_DESCRIPTION "\n"
    "\n"
    "THIS PROGRAM IS STILL JUST A PROTOTYPE, AND NOT\n"
    "ACTUALLY USEFUL YET\n"
    "\n"
    "OPTIONS:\n"
    "  -f file     set file\n"
    "  -v level    Set verbosity level [0-3]\n"
    "  -h          Show this message and exit\n"
    "  -V          Show version and exit\n"
    ;
//                                                         <max

void print_help(FILE * out, char* prog_name) {
    fprintf(out, help_msg, prog_name);
}

struct cli_options parse(int argc, char** argv) {
    struct cli_options options = new_options();
    int opt;
    while((opt = getopt(argc, argv, optstring)) != -1) {
        switch(opt) {
            case 'f':
                options.file.value = strdup(optarg);
                options.file.has_value = true;
                break;
            case 'v':
                options.verbosity = atoi(optarg);
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
