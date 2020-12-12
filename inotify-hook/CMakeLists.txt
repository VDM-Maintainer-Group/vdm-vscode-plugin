cmake_minimum_required(VERSION 3.12) #for FindPython feature
##
project(vdm-vscode-plugin VERSION 0.1)


## TODO: https://musteresel.github.io/posts/2020/02/cmake-template-linux-kernel-module.html
# KDIR := /lib/modules/$(shell uname -r)/build
# all:build
# build:
# 	$(MAKE) -C $(KDIR) M=$$PWD
# clean:
# 	$(MAKE) -C $(KDIR) M=$$PWD clean
# insmod:build
# 	sudo insmod inotify_hook.ko
# rmmod:
# 	sudo rmmod inotify_hook.ko
# dmesg:
# 	@sudo dmesg | grep --color "\[printk\]"