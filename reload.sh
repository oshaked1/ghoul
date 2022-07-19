#! /bin/bash
rmmod ghoul 2> /dev/null
make clean
make
insmod ghoul.ko