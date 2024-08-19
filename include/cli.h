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
    optional_str pipeline_log_dir;
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
