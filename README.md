# VDM Vscode Plugin

* The efficient solution exists in `inotify_group` reverse lookup, however, I could not find the way for now.
* So, I hook both the `add_watch` and `rm_watch` to maintain existing watcher list for each process.
    * With **GUI-only** and **blacklist** design, it would alleviate the overhead a lot.
* The maintained list would be available on *proc* subsystem, and easily retrieved from userspace.

### Reference
1. https://security.stackexchange.com/questions/210897/why-is-there-a-need-to-modify-system-call-tables-in-linux
2. https://stackoverflow.com/questions/2103315/linux-kernel-system-call-hooking-example
3. https://stackoverflow.com/questions/11915728/getting-user-process-pid-when-writing-linux-kernel-module
4. https://uwnthesis.wordpress.com/2016/12/26/basics-of-making-a-rootkit-from-syscall-to-hook/
5. https://stackoverflow.com/questions/58819136/is-it-possible-to-dump-inode-information-from-the-inotify-subsystem