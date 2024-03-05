#include <stdio.h>
#include <stdlib.h>

#include <stdnoreturn.h>
#include <string.h>

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
  {0, 0};\n\
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

int main(int argc, char **args) {
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

    

}
