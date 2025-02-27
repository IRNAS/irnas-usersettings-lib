# User Settings Bluetooth Serive

This sample demonstrates the User Settings Service (USS). Use the sample with a nrf52840dk.

The device will advertise as `USER_SETTINGS_BT`. The device the user settings bluetooth service. See
[bt_uss.h](../../library/include/bt_uss.h) for details on the service.

Write to the service to get/set settings. Use the shell to verify the changes.

## Building and using

Build the sample for the nrf52840DK:

```bash
east build -b nrf52840dk/nrf52840
```

Logs are available via UART. Use `minicom`, `tio` or a similar tool to connect to the device.

```bash
minicom -D /dev/ttyACM0 ## or ACM1, ACM2, etc.
```
