# INTERNAL

THIS IS AN "INTERNAL" SAMPLE. It demostrates some bit of the internals of the user-settings libary.
This means that is includes source and header files from within the library with CMake.
The functionality here can not be used via the library API. If this is ever required, the library must be refactored.

## Binary Encoding

This sample adds a setting of each type and gives them default values and values.
Then, it encodes each one using the binary encoder and prints the encoded buffer.