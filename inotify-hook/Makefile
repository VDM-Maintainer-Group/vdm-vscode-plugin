KDIR := /lib/modules/$(shell uname -r)/build

all:build

build:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

insmod:build
	sudo insmod inotify_hook.ko

rmmod:
	sudo rmmod inotify_hook.ko

dmesg:
	@sudo dmesg | grep --color "\[printk\]"
