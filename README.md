# KVDB - Vita Debugger Kernel Plugin

This plugin is used to implement the kernel-space functionality for debugging applications on the PSVita.

## Usage
Add this plugin to your `config.txt` under the `[KERNEL]` category. This plugin by default will listen on the UART port for GDB commands.

To use this plugin over TCP/IP then you will need the [vdbtcp companion plugin][1].

To use this plugin over UART, you need to load an application in debug mode prior to connecting GDB. This plugin does not load the target itself.

## Known Issues
Currently this software is in alpha testing. There are a number of issues when debugging an application such as:
* Only able to debug a single session per boot.
* Memory faults are not correctly handled.
* Not on this list? Create a bug report.

  [1]: https://github.com/DaveeFTW/vdbtcp