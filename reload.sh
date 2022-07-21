#! /bin/bash
python3 ghoulctl.py unload
make clean
make
insmod ghoul.ko