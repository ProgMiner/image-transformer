# image-transformer

A simple modular application for transforming images in 24-bit BMP format.

This repository was created for an article by the following link:
[https://habr.com/ru/post/529262/](https://habr.com/ru/post/529262/).

## Requirements

To build this app you need to have following tools:

- GNU gcc
- GNU Flex
- GNU Bison
- GNU make

## Build & run

To build it you need to run `make` in the root directory.
As result you will have an executable file `image-transformer`.

To get help just run `./image-transformer -h`.

Summary:
```sh
make # build
./image-transformer -h # run
./test.it test.bmp # run test script via executable
```

## Scripting language

An example script is located at [./test.it](test.it).

Scripts consist of comments started from `#` and transformations:

```python
# comment

transformation();
module.transformation_from_module();

transformation_with_args(
    1,      # integer value
    2.0,    # floating point value
    "3",    # string value
    four    # identifier (from module) value
);
```

Transformation with specified module will be loaded from shared objects.
Shared objects should have name in format `<module_prefix><module>.so` or `<module_prefix><module>`,
where module prefix is defined via program arguments.

If module is not specified transformation will be loaded from ["standard library"](stdlib.c).
There are some standard utilitary "transformations":

### `print_ansi([pixel])`
Print image on terminal with ANSI ESC-sequences.

`pixel` is an optional parameter, that specifies string that
will be used to print each pixel of image, default "  " (two spaces).

### `echo([message])`
Print message with newline.
If message is not specified, prints only newline.

### `die([message])`
Abort script with message.
If message is not specified, error message will be "suicide".

## Module writing

Modules are simple ELF shared objects with exported functions.

Transformation functions should have following signature:
```c
/* Header files from application source code */
#include <value.h>
#include <image.h>

const char * transformation_name(struct image * image, uint32_t argc, const struct value * argv) {
    /* transformation code */
}
```

To call transformation described above you need to write a code like the following one in script
and make sure that shared library can be found by an application:
```python
# Assume that "transformation_name" is compiled in "module_name.so" shared object
module_name.transformation_name();
```
