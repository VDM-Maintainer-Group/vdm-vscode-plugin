# VDM Vscode Plugin
- The efficient solution exists in `inotify_group` reverse lookup, however, there is no existing mechanism available.
- So, I hook both the `add_watch` and `rm_watch` to maintain existing watcher list for each process.
- To keep the kernel module clearness, the (PID, FD_WD) lookup table should be maintained in userspace.
- Netlink will response to request of PID from userspace in unicast (no encryption considered for now).

### Instruction
- `cmake>=3.12.0` (if not satisfied, try `pip3 install -U cmake` to get the latest version)

### TODO
- Verify the inotify mechanism (add/update/remove)
- Complete netlink unicast between kernel module and daemon.
- Add periodic garbage information collection based on alive process.
- Add DKMS compiling.
- Complete the de-facto vscode plugin.

### NOTES
- editors using inotify: `code`, `Typora`, `gedit`, `okular`, etc.

### Reference
1. https://security.stackexchange.com/questions/210897/why-is-there-a-need-to-modify-system-call-tables-in-linux
2. https://stackoverflow.com/questions/2103315/linux-kernel-system-call-hooking-example
3. https://stackoverflow.com/questions/11915728/getting-user-process-pid-when-writing-linux-kernel-module
4. https://uwnthesis.wordpress.com/2016/12/26/basics-of-making-a-rootkit-from-syscall-to-hook/
5. https://stackoverflow.com/questions/58819136/is-it-possible-to-dump-inode-information-from-the-inotify-subsystem
