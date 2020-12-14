/* Inotify WD Reverse Lookup Kernel Module
* References:
* 1. https://elixir.bootlin.com/linux/latest/source/include/linux/radix-tree.h
* 2. https://biscuitos.github.io/blog/IDA_ida_destroy/
* 3. https://lwn.net/Articles/175432/
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include "main.h"
#include "khook/khook/engine.c"
#include "netlink_comm.h"

/************************* PROTOTYPE DECLARATION *************************/
static int __init inotify_hook_init(void);
static void __exit inotify_hook_fini(void);

/***************************** UTILITY FUNCTION *****************************/


/*************************** INOTIFY SYSCALL HOOK ***************************/
// regs->(di, si, dx, r10), reference: arch/x86/include/asm/syscall_wrapper.h#L125
//asmlinkage long sys_inotify_add_watch(int fd, const char __user *path, u32 mask);
KHOOK_EXT(long, ORIGIN(inotify_add_watch), const struct pt_regs *);
static long MODIFY(inotify_add_watch)(const struct pt_regs *regs)
{
    int wd;
    struct path path;
    unsigned int flags = 0;
    char *pname = NULL;
    char buf[PATH_MAX];
    unsigned long usr_pid = task_pid_nr(current); //current->pid;
    // decode the registers
    int fd = (int) regs->di;
    const char __user *pathname = (char __user *) regs->si;
    u32 mask = (u32) regs->dx;

    wd = KHOOK_ORIGIN(ORIGIN(inotify_add_watch), regs);

    if (!(mask & IN_DONT_FOLLOW))
        flags |= LOOKUP_FOLLOW;
    if (mask & IN_ONLYDIR)
        flags |= LOOKUP_DIRECTORY;
    if ( wd>=0 && user_path_at(AT_FDCWD, pathname, flags, &path)==0 )
    {
        pname = dentry_path_raw(path.dentry, buf, PATH_MAX);
        path_put(&path);
        printh("PID %ld add: %s\n", usr_pid, pname);
    }

    return wd;
}

//asmlinkage long sys_inotify_rm_watch(int fd, __s32 wd);
KHOOK_EXT(long, __x64_sys_inotify_rm_watch, const struct pt_regs *regs);
static long khook___x64_sys_inotify_rm_watch(const struct pt_regs *regs)
{
    int ret;

    ret = KHOOK_ORIGIN(__x64_sys_inotify_rm_watch, regs);

    return ret;
}

/****************************** MAIN_ENTRY ******************************/
static int __init inotify_hook_init(void)
{
    int ret = 0;

    /* init khook engine */
    if ( (ret = khook_init()) < 0 )
    {
        return ret;
    }

    /* init netlink */
    // if ( netlink_comm_init < 0 )
    // {
    //     printh("Netlink_Comm Initialization Failed.\n");
    //     return -EINVAL;
    // }

    printh("Inotify hook module init.\n");
    return 0;
}

static void __exit inotify_hook_fini(void)
{
    // netlink_comm_exit();
    khook_cleanup();
    printh("Inotify hook module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");
