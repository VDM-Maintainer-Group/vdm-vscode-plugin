
all:dependency build install

dependency:
	pip3 install gnlpy --user

build:
	$(MAKE) -C inotify-hook

install:
	sudo cp -f inotify-hook/inotify_hook.ko /lib/modules/$(shell uname -r)/updates/inotify_hook.ko