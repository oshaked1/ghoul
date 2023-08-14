# Ghoul
Ghoul is an LKM rootkit for Linux, focusing primarily on stealth.

# How to Run
Install dependencies:
```
apt install build-essential make linux-headers-$(uname -r)
```

Use the `reload.sh` script to build and insert/reinsert the rootkit kernel module.

# Platform Support
Ghoul is currently being developed on Ubuntu server 20.04 with kernel version `5.4.0-125-generic`.
Other versions have yet to be tested.