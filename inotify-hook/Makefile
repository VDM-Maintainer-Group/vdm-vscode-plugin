ifneq ($(KERNELRELEASE),)
	obj-m := inotify_hook.o
	inotify_hook-objs := entry.o
else
	PWD := $(shell pwd)
	KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD)

insmod:
	sudo insmod inotify_hook.ko

rmmod:
	sudo rmmod inotify_hook.ko

dmesg:
	@sudo dmesg | grep --color "\[printk\]"

clean:
	rm -f .cache.mk .*.cmd
	rm -f *.o *.o.cmd *.ko *.mod.c *.symvers *.order
	rm -rf .tmp_versions

endif