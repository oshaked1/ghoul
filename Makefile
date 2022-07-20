obj-m += ghoul.o
ghoul-objs := ./src/ghoul.o ./src/load.o ./src/hooks.o ./src/service.o
KVERSION = $(shell uname -r)
all:
		make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
		make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean