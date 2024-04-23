TARGET_MODULE:=sdrv

obj-m := $(TARGET_MODULE).o

# If we are running without kernel build system
ifeq ($(KERNELRELEASE),)
	KDIR?=/lib/modules/$(shell uname -r)/build
	PWD:=$(shell pwd)


all : 
# run kernel build system to make module
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
# run kernel build system to cleanup in current directory
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	insmod ./$(TARGET_MODULE).ko

unload:
	rmmod ./$(TARGET_MODULE).ko

endif

