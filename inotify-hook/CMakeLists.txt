cmake_minimum_required(VERSION 3.12) #for FindPython feature
##
project(vdm-inotify-hook VERSION 0.1)
execute_process(COMMAND uname -r OUTPUT_VARIABLE KERNVER
                OUTPUT_STRIP_TRAILING_WHITESPACE)
set(KDIR "/lib/modules/${KERNVER}/build")
# set(INSTALL_DIRECTORY "/lib/modules/${KERNVER}/updates") #inotify_hook.ko


## directly call the Kbuild
# TODO: https://musteresel.github.io/posts/2020/02/cmake-template-linux-kernel-module.html

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
