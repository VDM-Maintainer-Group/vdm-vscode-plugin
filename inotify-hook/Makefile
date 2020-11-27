ifneq ($(KERNELRELEASE),)
	obj-m := inotify_hook.o
	inotify_hook-objs := main.o
	# inotify_hook-objs := main.o netlink_comm.o
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
	rm -f .cache.mk .*.cmd built-in.a
	rm -f *.o *.o.cmd *.ko *.mod *.mod.c *.symvers *.order
	rm -rf .tmp_versions

endif