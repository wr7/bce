bce (Binary C Embed) is a simple program for converting a file to a C byte array. This can be used with build systems like Meson or Make to embed binary files directly into a C program.
`bce` is meant to be a lightweight replacement for `vim`'s `xxd -i` command, but `bce`'s output is simpler/different, so it is **not** a drop-in replacement for `xxd -i`.
### Installing
`bce` was designed so that its `bce.c` file can be symlinked or copied directly into your project. This way, your project can architecture-independently ship `bce` around with it and not require it as an external dependency.
To do this with Meson, see the [Using with Meson](#using-with-meson) section.

`bce` can also be compiled and used as a standalone application. Since `bce` has only one file and no dependencies, it can be directly compiled with:
```bash
gcc bce.c -o bce
```

For easier development, some Nix, Meson, and Bash files have been provided. To compile using these (assuming you have Nix installed), you can do
```bash
nix-shell --pure
source envsetup.sh
b
```


### Usage
`bce <input_file> <output_file>`

To read from `stdin` or output to `stdout`, `-` can be used as an input or output file name.

`bce` does not have any flags due to its simplified output.

### Differences from `xxd -i`
`bce` has different output compared to `xxd -i`.
Say the user runs `bce foo.bin foo.h`. 
If `foo.bin` contains two null bytes, the generated `foo.h` will contain:
```c
{0,0,};
```

Unlike `xxd -i`, the user must create a variable to store the data themselves,
and they must find the length of the data themselves. 

This can be done with the following C code:
```c
const char FOO[] =
#include "foo.h"

const size_t FOO_LEN = sizeof(FOO) / sizeof(FOO[0]);
```

### Using with Meson
The following examples assume that the `bce.c` file has been copied into the same directory as your project's `build.meson` file.

#### Simple example
build.meson:
```meson
bce_prog = executable('bce', 'bce.c')

bce = generator(bce_prog,
                output  : '@BASENAME@.h',
                arguments : ['@INPUT@', '@OUTPUT@'])

embedded_files = [
  'foo_icon.png',
  'bar_icon.png'
]

project_source_files = [
  # Omitted... #
]

embed_headers = bce.process(embedded_files)

exe = executable('my_project', project_source_files + embed_headers,
  install : false)
```
C code:
```c
const unsigned char FOO_ICON[] =
#include "foo_icon.h"

const unsigned char BAR_ICON[] =
#include "bar_icon.h"
```

#### Real use case: compiling and embedding GLSL shaders
When writing C applications with Vulkan, you will need SPIR-V shaders.
With the following code, Meson will automatically recompile your shaders into SPIR-V and embed them into your project whenever their source code is updated.

build.meson:
```meson
glslc_prog = find_program('glslc')
bce_prog = executable('bce', 'bce.c')

glslc = generator(glslc_prog,
                output  : '@BASENAME@.spv',
                arguments : ['@INPUT@', '-o', '@OUTPUT@'])

bce = generator(bce_prog,
                output  : '@BASENAME@.h',
                arguments : ['@INPUT@', '@OUTPUT@'])

shader_source_files = [
  'src/vertex_shader.vert',
  'src/fragment_shader.frag'
]

project_source_files = [
  # Omitted... #
]

compiled_shaders = glslc.process(shader_source_files)
shader_headers = bce.process(compiled_shaders)

exe = executable('my_vulkan_project', project_source_files + shader_headers,
  install : false, dependencies : [vulkan, glfw3])
```
C code:
```c
// Vulkan requires that shaders are 32-bit aligned //
__attribute__( ( aligned ( 4 ) ) )
const unsigned char VERTEX_SHADER[] =
#include "vertex_shader.h"

__attribute__( ( aligned ( 4 ) ) )
const unsigned char FRAGMENT_SHADER[] =
#include "fragment_shader.h"
```
