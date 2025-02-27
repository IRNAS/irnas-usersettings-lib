# INTERNAL

THIS IS AN "INTERNAL" SAMPLE. It demonstrates some bit of the internals of the user-settings
library. This means that is includes source and header files from within the library with CMake. The
functionality here can not be used via the library API. If this is ever required, the library must
be refactored.

## Binary Encoding

This sample adds a setting of each type and gives them default values and values. Then, it encodes
each one using the binary encoder and prints the encoded buffer.

## Building and using

Build the sample for the native platform and run it:

```bash
east build -b native_sim
./build/binary_encoding/zephyr/zephyr.exe
```

The first line printed will tell you which pseudo tty the device has attached to. For example:

```bash
uart connected to pseudotty: /dev/pts/11
```

Use `minicom`, `tio` or a similar tool to connect to the pseudo tty:

```bash
minicom -D /dev/pts/11
```

Use the `usettings` shell command to interact with the settings.
