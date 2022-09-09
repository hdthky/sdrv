TARGET_MODULE:=scdd

obj-m := $(TARGET_MODULE).o

# If we are running without kernel build system
ifeq ($(KERNELRELEASE),)
	BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
	PWD:=$(shell pwd)


all : 
# run kernel build system to make module
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules

clean:
# run kernel build system to cleanup in current directory
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean

load:
	lsmod | grep -q $(TARGET_MODULE) || insmod ./$(TARGET_MODULE).ko
	if [ ! -c /dev/$(TARGET_MODULE) ];\
	then mknod /dev/$(TARGET_MODULE) c 236 0; chmod 666 /dev/$(TARGET_MODULE);\
	else rm /dev/$(TARGET_MODULE); mknod /dev/$(TARGET_MODULE) c 236 0; chmod 666 /dev/$(TARGET_MODULE); fi

unload:
	if [ -c /dev/$(TARGET_MODULE) ]; then rm /dev/$(TARGET_MODULE); fi
	lsmod | grep -q $(TARGET_MODULE) && rmmod ./$(TARGET_MODULE).ko || true

endif

