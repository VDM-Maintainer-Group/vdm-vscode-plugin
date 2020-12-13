cmake_minimum_required(VERSION 3.12) #for FindPython feature
##
project(vdm-inotify-hook VERSION 0.1)
set(KDIR "" CACHE STRING "/lib/modules/$(shell uname -r)/build")
# set(INSTALL_DIRECTORY "lib/modules/`shell uname -r`/updates") #inotify_hook.ko


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
