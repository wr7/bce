#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdbool.h>

#include <errno.h>

static void print_usage(void) {
    fprintf(stderr, "\
Converts a file to a C byte array. \n\
This is a non drop-in replacement for `xxd -i`.\n\
\n\
Usage\n\
bce <input_file> <output_file>\n\
`-` can be used in place of <input_file> or <output_file>\n\
to read from stdin or to output to stdout\n\
\n\
Ouput\n\
bce has very different output compared to `xxd -i`.\n\
Say the user runs `bce foo.bin foo.h`. \n\
If foo.bin contains only two null bytes, bce will only output:\n\
  {0,0,};\n\
\n\
Unlike xxd, the user must create the variable storing the data themselves,\n\
and they must find the length themselves. \n\
\n\
This can be done with the following C code:\n\
\n\
const char FOO[] =\n\
#include \"foo.h\"\n\
\n\
const size_t FOO_LEN = sizeof(FOO)/sizeof(FOO[0]);\n\
");
}

static void check_write_error() {
    if(errno != 0) {
        perror("bce: failed to write to output file");
        exit(errno);
    }
}

int main(int argc, char **args) {
    // Check commandline arguments //
    if(argc == 2) {
        if(strcmp(args[1], "--help") == 0 || strcmp(args[1], "-h") == 0) {
            print_usage();
            return 0;
        }
    }

    if(argc != 3) {
        fprintf(stderr, "bce: Invalid usage, see `bce --help` for more information.");
        return 1;
    }

    // Open input and output files //
    const char *const input_file_name = args[1];
    FILE *input_file;
    if(strcmp(input_file_name, "-") == 0) {
        input_file = stdin;
    } else {
        input_file = fopen(input_file_name, "r");
    }
    if(input_file == NULL) {
        perror("bce: Failed to open input file");
        exit(errno);
    }

    const char *const output_file_name = args[2];
    FILE *output_file;
    if(strcmp(output_file_name, "-") == 0) {
        output_file = stdout;
    } else {
        output_file = fopen(output_file_name, "w");
    }
    if(output_file == NULL) {
        perror("bce: Failed to open output file");
        exit(errno);
    }

    // Read and write to input/output files //
    errno = 0;
    fprintf(output_file, "{");
    check_write_error();

    for(int byte = getc(input_file); byte != EOF; byte = getc(input_file)) {
        fprintf(output_file, "%u,", byte);
        check_write_error();
    }

    if(errno != 0) {
        perror("bce: Failed to read from input file");
    }

    fprintf(output_file, "};");
    check_write_error();

    fclose(output_file);
    fclose(input_file);
}
