# User Settings callbacks

Sample to demonstrate the use of `on_change` and `validate` callbacks of the user_settings lib.

## Building and using

Build the sample for the native platform and run it:

```bash
east build -b native_sim
./build/callbacks/zephyr/zephyr.exe
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
