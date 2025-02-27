# Validation

Sample to demonstrate the validation helpers of the user_settings lib.

To see how a normal validation callback might look (without using a helper macro), check the
`callbacks` sample.

## Building and using

Build the sample for the native platform and run it:

```bash
east build -b native_sim
./build/validation/zephyr/zephyr.exe
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
