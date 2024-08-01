#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>

struct cli_options {
    int verbosity;
    bool help;
    bool version;
};

struct cli_options new_options() {
    struct cli_options result;
    result.verbosity = 0;
    result.help = false;
    return result;
}

//                                                         <max
char* help_msg = 
    "Usage: %s [-v level] [-h] [-V]\n"
    "\n"
    SCI_NAME " is a simple contiuous integration system.\n"
    "\n"
    "OPTIONS:\n"
    "  -v level    Set verbosity level [0-3]\n"
    "  -h          Show this message and exit\n"
    "  -V          Show version and exit\n"
    ;
//                                                         <max

void print_help(FILE * out, char* prog_name) {
    fprintf(out, help_msg, prog_name);
}

int main(int argc, char** argv) {
    struct cli_options options = new_options();
    int opt;
    while((opt = getopt(argc, argv, "v:hV")) != -1) {
        switch(opt) {
            case 'v':
                options.verbosity = atoi(optarg);
                break;
            case 'V':
                options.version = true;
                break;
            case 'h':
                options.help = true;
                break;
            default: // '?'
                print_help(stderr, argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if(options.help) {
        print_help(stdout, argv[0]);
        exit(EXIT_SUCCESS);
    }
    if(options.version) {
        fprintf(stdout, SCI_VERSION "\n");
        exit(EXIT_SUCCESS);
    }
}
