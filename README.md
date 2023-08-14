# Ghoul
Ghoul is an LKM rootkit for Linux, focusing primarily on stealth.

# How to Run
Install dependencies:
```
apt install build-essential make linux-headers-$(uname -r)
```

To build, use `make`. To build and load the rootkit, use `make run` (will prompt for sudo credentials).

To build and run all tests, use `make test`. To build and run rkhunter (an open-source rootkit detection tool) use `make rkhunter`.

# Controlling the Rootkit
Ghoul receives service requests (commands) from the user via the `ioctl` syscall, with the predefined fd number 666 (can be changed at runtime).

The ioctl `cmd` argument specifis the requested service and the `arg` argument contains parameters specific to that service request.

Some services may require multiple parameters or parameters that can't fit into the `unsigned long` type of `arg`.
In such cases, a user mode pointer is supplied which points to a struct with arguments.

To simplify sending service requests to ghoul, the `ghoulctl.py` utility is provided which allows sending service requests via the command line.

A list of services and how to use them can be found [here](./SERVICE_REQUESTS.md)

# Stealth
As noted, the main focus of this rootkit is stealth. The rootkit currently hides from the following artifacts:
- procfs (`/proc/modules`) - used by `lsmod`
- sysfs (`/sys/module`)
- ftrace function tracer output
- ftrace hooked functions list (`/sys/kernel/debug/tracing/enabled_functions`)

# Platform Support
Ghoul currently supports only x86_64 but support for x86 is planned. ARM support may be added later.

In terms of kernel versions, ghoul is currently being developed on Ubuntu server 20.04 with kernel version `5.4.0-125-generic`.
Other versions may work but have yet to be tested.