
all:build install

build:build-krn build-usr

build-krn:
	$(MAKE) -C inotify-hook

build-usr:

install:
	sudo cp -f inotify-hook/inotify_hook.ko /lib/modules/$(shell uname -r)/updates/inotify_hook.ko