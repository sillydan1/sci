#ifndef SCI_CLI_H
#define SCI_CLI_H
#include "optional.h"
#include <stdio.h>

typedef struct {
    optional_str config_file;
    int executors;
    int verbosity;
    bool help;
    bool version;
    bool use_colors;
    optional_str log_file;
} cli_options;

// Construct a new cli_options struct instance.
cli_options new_options();

// Delete a cli_options struct instance.
void destroy_options(cli_options v);

// Print the help message.
void print_help(FILE * out, char* prog_name);

// Parse the command line arguments and give a new cli_options struct instance.
cli_options parse(int argc, char** argv);

#endif
