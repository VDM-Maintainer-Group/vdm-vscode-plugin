# VDM Vscode Plugin
* The efficient solution exists in `inotify_group` reverse lookup, however, there is no existing mechanism available.
* So, I hook both the `add_watch` and `rm_watch` to maintain existing watcher list for each process.
* Meanwhile, the `sys_execve`/`sys_fork`/`sys_exit_group` are all hooked to associate with inotify table, and two-level radix_tree is leveraged to maintain (PID,FD_WD) fast lookup table in kernel.
* Netlink will response to request of PID from userspace in unicast (no encryption considered).

### TODO
* Fragment netlink sending in kernel, and finish corresponding receiving in usersapce.
* Complete the de-facto vscode plugin.

### Reference
1. https://security.stackexchange.com/questions/210897/why-is-there-a-need-to-modify-system-call-tables-in-linux
2. https://stackoverflow.com/questions/2103315/linux-kernel-system-call-hooking-example
3. https://stackoverflow.com/questions/11915728/getting-user-process-pid-when-writing-linux-kernel-module
4. https://uwnthesis.wordpress.com/2016/12/26/basics-of-making-a-rootkit-from-syscall-to-hook/
5. https://stackoverflow.com/questions/58819136/is-it-possible-to-dump-inode-information-from-the-inotify-subsystem
