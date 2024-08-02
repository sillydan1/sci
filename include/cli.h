#ifndef SCI_CLI_H
#define SCI_CLI_H
#include <stdio.h>
#include "optional.h"

struct cli_options {
    optional_str file;
    int verbosity;
    bool help;
    bool version;
};

// Construct a new cli_options struct instance.
struct cli_options new_options();

// Delete a cli_options struct instance.
void free_options(struct cli_options v);

// Print the help message.
void print_help(FILE * out, char* prog_name);

// Parse the command line arguments and give a new cli_options struct instance.
struct cli_options parse(int argc, char** argv);

#endif
