# VDM VScode Plugin
- The efficient solution exists in `inotify_group` reverse lookup, however, there is no existing mechanism available.
- So, I hook both the `inotify_add_watch` and `inotify_rm_watch` to maintain existing watcher list for each process.
- Netlink will response to request of `comm_name` from userspace in unicast (no encryption considered for now).

### Instruction
- `cmake>=3.12.0` (if not satisfied, try `pip3 install -U cmake` to get the latest version)

### TODO
- ~~Verify the inotify mechanism (add/update/remove)~~
- ~~Add *global linked-list* for interested **comm_name** register~~
    - `int comm_list_find(const char *)`
    - `int comm_list_add(const char *)`
    - `void comm_list_rm(const char *)`
    - `int comm_list_init(void)`, `void comm_list_exit(void)`
- ~~Complete **comm_name_list** call in three hooked syscalls (init with `code`)~~
- Add *in-kernel radix tree* for inode recording
    - `(char[MAX_NAME_LEN]) comm_name -> (pid_tree *) -> (struct inode_record *)(int fd, int wd, char *pathname)`
    - `int radix_tree_init()`, `void radix_tree_reset()`
    - `int radix_tree_insert(const char *comm_name, pid_tree *, struct inode_record *)`
    - `void radix_tree_remove(const char *comm_name, pid_tree *, const struct inode_record *)`
    - `void radix_tree_remove_by_id(const char *comm_name, pid_tree *, int fd, int wd)` //wrapper function
    - `(struct inode_record **) radix_tree_dump(const char *comm_name)`
- Add *serialization function* for `struct inode_record **`
- Complete **radix_tree** modify in three hooked system syscalls
- Add netlink unicast request-response model in the module
    
    - > TODO
- Complete *netlink function* call in the model
- Complete netlink daemon
- Add DKMS compiling
- Complete the de-facto vscode plugin

### NOTES
- editors using inotify: `code`, `Typora`, `okular`, etc.

### Reference
1. https://security.stackexchange.com/questions/210897/why-is-there-a-need-to-modify-system-call-tables-in-linux
2. https://stackoverflow.com/questions/2103315/linux-kernel-system-call-hooking-example
3. https://stackoverflow.com/questions/11915728/getting-user-process-pid-when-writing-linux-kernel-module
4. https://uwnthesis.wordpress.com/2016/12/26/basics-of-making-a-rootkit-from-syscall-to-hook/
5. https://stackoverflow.com/questions/58819136/is-it-possible-to-dump-inode-information-from-the-inotify-subsystem
