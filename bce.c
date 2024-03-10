/*
   bce v1.1

   https://github.com/wr7/bce

   bce is software for converting binary files to C headers. This is useful for 
   embedding binary files into your C project.

   This software is licensed under the MIT license.

   Copyright © 2024 wr7

   Permission is hereby granted, free of charge, to any person obtaining a copy 
   of this software and associated documentation files (the “Software”), to 
   deal in the Software without restriction, including without limitation the 
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
   sell copies of the Software, and to permit persons to whom the Software is 
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in 
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
   IN THE SOFTWARE.
 */

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
