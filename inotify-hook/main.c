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
#include "khook/engine.c"
#include "main.h"
// #include "netlink_comm.h"


/************************* PROTOTYPE DECLARATION *************************/
static int __init inotify_hook_init(void);
static void __exit inotify_hook_fini(void);

/*************************** INOTIFY SYSCALL HOOK ***************************/
// regs->(di, si, dx, r10), reference: arch/x86/include/asm/syscall_wrapper.h#L125
//asmlinkage long sys_inotify_add_watch(int fd, const char __user *path, u32 mask);
KHOOK_EXT(long, __x64_sys_inotify_add_watch, const struct pt_regs *);
static long khook___x64_sys_inotify_add_watch(const struct pt_regs *regs)
{
    int wd;
    struct path path;
    unsigned int flags = 0;
    char *pname = NULL;
    // char *precord = NULL;
    char buf[PATH_MAX];
    unsigned long usr_pid = task_pid_nr(current); //current->pid;
    // struct radix_tree_root *wd_table;
    // decode the registers
    int fd = (int) regs->di;
    const char __user *pathname = (char __user *) regs->si;
    u32 mask = (u32) regs->dx;

    wd = KHOOK_ORIGIN(__x64_sys_inotify_add_watch, regs);

    if (!(mask & IN_DONT_FOLLOW))
        flags |= LOOKUP_FOLLOW;
    if (mask & IN_ONLYDIR)
        flags |= LOOKUP_DIRECTORY;
    
    if ( user_path_at(AT_FDCWD, pathname, flags, &path) == 0 )
    // if (wd>=0 && user_path_at(AT_FDCWD, pathname, flags, &path)==0)
    {
        pname = dentry_path_raw(path.dentry, buf, PATH_MAX);
        path_put(&path);
        printh("PID %ld add: %s\n", usr_pid, pname);

        // wd_table = radix_tree_lookup(PID_TABLE, usr_pid);
        // if (wd_table==NULL)
        // {
        //     printh("PID %d: (add_watch) No Record Found.\n", usr_pid);
        //     return;
        // }
        
        // //NOTE: insert inotify record for PID in PID_TABLE
        // precord = kmalloc(strlen(pname), GFP_ATOMIC);
        // strcpy(precord, pname);
        // if (fd>=1000) //at most 1000 fd allowed
        // {
        //     printh("PID %d: Record Up Limit Achieved.\n", usr_pid);
        // }
        // else
        // {
        //     radix_tree_insert(wd_table, wd*1000+fd, precord)
        // }
    }

    printh("%d called add_watch\n", fd);

    return wd;
}

//asmlinkage long sys_inotify_rm_watch(int fd, __s32 wd);
KHOOK_EXT(long, __x64_sys_inotify_rm_watch, const struct pt_regs *regs);
static long khook___x64_sys_inotify_rm_watch(const struct pt_regs *regs)
{
    int ret;
    // char *precord;
    // struct radix_tree_root *wd_table;

    ret = KHOOK_ORIGIN(__x64_sys_inotify_rm_watch, regs);

    // wd_table = radix_tree_lookup(PID_TABLE, usr_pid);
    // if (wd_table==NULL)
    // {
    //     printh("PID %d: (rm_watch) No Record Found.\n", usr_pid);
    //     return;
    // }
    
    // remove inotify record for PID in PID_TABLE
    // precord = radix_tree_delete(wd_table, wd*1000+fd);
    // if (!(precord==NULL))
    // {
    //     kfree(precord);
    // }

    return ret;
}

/***************************** UTILITY FUNCTION *****************************/
// static void pid_tree_add(unsigned long pid)
// {
//     int error;
//     struct radix_tree_root *wd_table = kmalloc(sizeof(struct radix_tree_root), GFP_KERNEL);

//     INIT_RADIX_TREE(wd_table, GFP_ATOMIC);
//     error = radix_tree_insert(PID_TABLE, pid, (void *)wd_table);
//     if (error < 0)
//     {
//         printh("PID %d: Record Allocation Failed.\n", pid);
//     }
// }
// static void pid_tree_remove_item(struct radix_tree_root *item)
// {
//     struct radix_tree_iter iter;
//     void **slot;

//     radix_tree_for_each_slot(slot, item, &iter, 0){
//         void *t = radix_tree_deref_slot(slot);
//         if (!radix_tree_exception(t))
//             kfree(t);
//         radix_tree_iter_delete(PID_TABLE, &iter, slot);
//     }
//     radix_tree_delete(PID_TABLE, pid);
//     kfree(wd_table);
// }
// static void pid_tree_remove(unsigned long pid)
// {
//     struct radix_tree_root *wd_table;

//     wd_table = (struct radix_tree_root *) radix_tree_lookup(PID_TABLE, pid);
//     if (wd_table==NULL)
//     {
//         return;
//     }
//     else{
//         pid_tree_remove_item(wd_table);
//     }
// }
// static void pid_tree_clean(void)
// {
//     struct radix_tree_iter iter;
//     void **slot;

//     radix_tree_for_each_slot(slot, PID_TABLE, &iter, 0){
//         void *item = radix_tree_deref_slot(slot);
//         if (!radix_tree_exception(item))
//             pid_tree_remove_item(item);
//         radix_tree_iter_delete(PID_TABLE, &iter, slot);
//     }
// }

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
    // pid_tree_clean();
    khook_cleanup();
    printh("Inotify hook module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");
