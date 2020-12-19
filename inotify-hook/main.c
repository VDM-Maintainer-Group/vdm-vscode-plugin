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
//regs->(di, si, dx, r10), reference: arch/x86/include/asm/syscall_wrapper.h#L125
//SYSCALL_DEFINE3(inotify_add_watch, int, fd, const char __user *, pathname, u32, mask)
KHOOK_EXT(long, ORIGIN(inotify_add_watch), const struct pt_regs *);
static long MODIFY(inotify_add_watch)(const struct pt_regs *regs)
{
    int wd;
    struct path path;
    unsigned int flags = 0;
    char *pname = NULL;
    char buf[PATH_MAX];
    unsigned int usr_pid = task_pid_nr(current); //current->pid;
    // decode the registers
    int fd = (int) regs->di;
    const char __user *pathname = (char __user *) regs->si;
    u32 mask = (u32) regs->dx;

    // do the original function
    wd = KHOOK_ORIGIN(ORIGIN(inotify_add_watch), regs);
    // get the pathname
    if (!(mask & IN_DONT_FOLLOW))
        flags |= LOOKUP_FOLLOW;
    if (mask & IN_ONLYDIR)
        flags |= LOOKUP_DIRECTORY;
    if ( wd>=0 && user_path_at(AT_FDCWD, pathname, flags, &path)==0 )
    {
        pname = dentry_path_raw(path.dentry, buf, PATH_MAX);
        path_put(&path);
        printh("%s, PID %d add (%d,%d): %s\n", current->comm, usr_pid, fd, wd, pname);
    }

    //TODO: if pid or application name in interest list
    return wd;
}

//SYSCALL_DEFINE2(inotify_rm_watch, int, fd, __s32, wd)
KHOOK_EXT(long, ORIGIN(inotify_rm_watch), const struct pt_regs *regs);
static long MODIFY(inotify_rm_watch)(const struct pt_regs *regs)
{
    int ret;
    // decode the registers
    int fd = (int) regs->di;
    u32 wd = (u32) regs->si;

    ret = KHOOK_ORIGIN(ORIGIN(inotify_rm_watch), regs);

    printh("%s, PID %d remove (%d,%d)\n", current->comm, task_pid_nr(current), fd, wd);

    return ret;
}

//SYSCALL_DEFINE1(exit_group, int, error_code)
KHOOK_EXT(long, ORIGIN(exit_group), const struct pt_regs *regs);
static long MODIFY(exit_group)(const struct pt_regs *regs)
{
    int ret;
    // decode the registers
    int error_code = (int) regs->di;

    ret = KHOOK_ORIGIN(ORIGIN(exit_group), regs);

    printh("%s, PID %d exit with code:%d\n", current->comm, task_pid_nr(current), error_code);

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
