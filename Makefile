TARGET_MODULE := scdd
DEV_NAME := scd

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
	insmod $(TARGET_MODULE).ko
	if [ ! -c /dev/$(DEV_NAME) ];\
	then mknod /dev/$(DEV_NAME) c 236 0; chmod 666 /dev/$(DEV_NAME);\
	else rm /dev/$(DEV_NAME); mknod /dev/$(DEV_NAME) c 236 0; chmod 666 /dev/$(DEV_NAME); fi

unload:
	if [ -c /dev/$(DEV_NAME) ]; then rm /dev/$(DEV_NAME); fi
	rmmod $(TARGET_MODULE)

endif

