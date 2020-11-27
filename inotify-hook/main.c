/* Inotify WD Reverse Lookup Kernel Module
* References:
* 1. https://elixir.bootlin.com/linux/latest/source/include/linux/radix-tree.h
* 2. https://biscuitos.github.io/blog/IDA_ida_destroy/
* 3. https://lwn.net/Articles/175432/
*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include "main.h"
// #include "netlink_comm.h"


/************************* PROTOTYPE DECLARATION *************************/
void **p_sys_call_table;
asmlinkage long (*ori_inotify_add_watch) (int, const char __user *, u32);
asmlinkage long (*ori_inotify_rm_watch) (int, __s32);
// asmlinkage long (*ori_sys_execve) (const char __user *, const char __user *const __user *, const char __user *const __user *);
// asmlinkage long (*ori_sys_fork) (void);
// asmlinkage long (*ori_sys_exit_group)(int);

asmlinkage long mod_inotify_add_watch(int, const char __user *, u32);
asmlinkage long mod_inotify_rm_watch(int, __s32);
// asmlinkage long mod_sys_execve(const char __user *, const char __user *const __user *, const char __user *const __user *);
// asmlinkage long mod_sys_fork(void);
// asmlinkage long mod_sys_exit_group(int);

// static void pid_tree_add(unsigned long);
// static void pid_tree_remove(unsigned long);
// static void pid_tree_clean(void);
static int __init inotify_hook_init(void);
static void __exit inotify_hook_fini(void);


/*************************** PROCESS SYSCALL HOOK ***************************/
// asmlinkage long mod_sys_execve(const char __user *filename, const char __user *const __user *argv, const char __user *const __user *envp)
// {
//     pid_tree_add( (long)current->pid );
//     return ori_sys_execve(filename, argv, envp);
// }
// asmlinkage long mod_sys_fork(void)
// {
//     pid_tree_add( (long)current->pid );
//     return ori_sys_fork();
// }
// asmlinkage long mod_sys_exit_group(int error_code)
// {
//     pid_tree_remove( (long)current->pid );
//     return ori_sys_exit_group(error_code);
// }

/*************************** INOTIFY SYSCALL HOOK ***************************/
asmlinkage long mod_inotify_add_watch(int fd, const char __user *pathname, u32 mask)
{
    int wd;
    struct path path;
    unsigned int flags = 0;
    char *pname = NULL;
    char *precord = NULL;
    char buf[PATH_MAX];
    unsigned long usr_pid = current->pid;
    // struct radix_tree_root *wd_table;

    wd = ori_inotify_add_watch(fd, pathname, mask);

    if (!(mask & IN_DONT_FOLLOW))
        flags |= LOOKUP_FOLLOW;
    if (mask & IN_ONLYDIR)
        flags |= LOOKUP_DIRECTORY;
    if (wd>=0 && user_path_at(AT_FDCWD, pathname, flags, &path)==0)
    {
        pname = dentry_path_raw(path.dentry, buf, PATH_MAX);
        path_put(&path);

        // wd_table = radix_tree_lookup(PID_TABLE, usr_pid);
        // if (wd_table==NULL)
        // {
        //     printh("PID %d: (add_watch) No Record Found.\n", usr_pid);
        //     return;
        // }
        
        // //NOTE: insert inotify record for PID in PID_TABLE
        precord = kmalloc(strlen(pname), GFP_ATOMIC);
        strcpy(precord, pname);
        // if (fd>=1000) //at most 1000 fd allowed
        // {
        //     printh("PID %d: Record Up Limit Achieved.\n", usr_pid);
        // }
        // else
        // {
        //     radix_tree_insert(wd_table, wd*1000+fd, precord)
        // }
    }

    return wd;
}
asmlinkage long mod_inotify_rm_watch(int fd, __s32 wd)
{
    int ret;
    char *precord;
    // struct radix_tree_root *wd_table;

    ret = ori_inotify_rm_watch(fd, wd);

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
    /* initialization */
    // INIT_RADIX_TREE(wd_table, GFP_ATOMIC);
    // if ( netlink_comm_init < 0 )
    // {
    //     printh("Netlink_Comm Initialization Failed.\n");
    //     return -EINVAL;
    // }

    /* get sys_call_table pointer */
    p_sys_call_table = (void **) kallsyms_lookup_name("sys_call_table");
    if (!p_sys_call_table)
    {
        printh("Cannot find sys_call_table address.\n");
        return -EINVAL;
    }

    set_addr_rw((unsigned long)p_sys_call_table);
    ori_inotify_add_watch = p_sys_call_table[__NR_inotify_add_watch];
    ori_inotify_rm_watch  = p_sys_call_table[__NR_inotify_rm_watch];
    // ori_sys_execve        = p_sys_call_table[__NR_sys_execve];
    // ori_sys_fork          = p_sys_call_table[__NR_sys_fork];
    // ori_sys_exit_group    = p_sys_call_table[__NR_sys_exit_group];
    p_sys_call_table[__NR_inotify_add_watch] = mod_inotify_add_watch;
    p_sys_call_table[__NR_inotify_rm_watch]  = mod_inotify_rm_watch;
    // p_sys_call_table[__NR_sys_execve]        = mod_sys_execve;
    // p_sys_call_table[__NR_sys_fork]          = mod_sys_fork;
    // p_sys_call_table[__NR_sys_exit_group]    = mod_sys_exit_group;
    set_addr_ro((unsigned long)p_sys_call_table);

    printh("Inotify hook module init.\n");
    return 0;
}

static void __exit inotify_hook_fini(void)
{
    set_addr_rw((unsigned long)p_sys_call_table);
    p_sys_call_table[__NR_inotify_add_watch] = ori_inotify_add_watch;
    p_sys_call_table[__NR_inotify_rm_watch]  = ori_inotify_rm_watch;
    // p_sys_call_table[__NR_sys_execve]        = ori_sys_execve;
    // p_sys_call_table[__NR_sys_fork]          = ori_sys_fork;
    // p_sys_call_table[__NR_sys_exit_group]    = ori_sys_exit_group;
    set_addr_ro((unsigned long)p_sys_call_table);

    // netlink_comm_exit();
    // pid_tree_clean();
    printh("Inotify hook module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");
