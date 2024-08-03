#ifndef SCI_CLI_H
#define SCI_CLI_H
#include <stdio.h>
#include "optional.h"

typedef struct {
    // prototyping
    optional_str file;
    optional_str file2;

    // actual
    int verbosity;
    bool help;
    bool version;
    bool use_colors;
    optional_str log_file;
} cli_options;

// Construct a new cli_options struct instance.
cli_options new_options();

// Delete a cli_options struct instance.
void free_options(cli_options v);

// Print the help message.
void print_help(FILE * out, char* prog_name);

// Parse the command line arguments and give a new cli_options struct instance.
cli_options parse(int argc, char** argv);

#endif
