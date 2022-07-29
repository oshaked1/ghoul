#! /bin/bash
module_name=${GHOUL_MODULE_NAME:-ghoul}
python3 ghoulctl.py unload
make clean
make
sudo insmod $module_name.ko