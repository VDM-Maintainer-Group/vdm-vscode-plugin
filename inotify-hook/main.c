#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include "main.h"
#include "netlink_comm.h"


/************************* PROTOTYPE DECLARATION *************************/
RADIX_TREE(PID_TABLE, GFP_ATOMIC);

void **p_sys_call_table;
asmlinkage long (*ori_inotify_add_watch) (int, const char __user *, u32);
asmlinkage long (*ori_inotify_rm_watch) (int, __s32);
asmlinkage long (*ori_sys_execve) (const char __user *, const char __user *const __user *, const char __user *const __user *);
asmlinkage long (*ori_sys_fork) (void);
asmlinkage long (*ori_sys_exit_group)(int);

asmlinkage long mod_inotify_add_watch(int, const char __user *, u32);
asmlinkage long mod_inotify_rm_watch(int, __s32);
asmlinkage long mod_sys_execve(const char __user *, const char __user *const __user *, const char __user *const __user *);
asmlinkage long mod_sys_fork(void);
asmlinkage long mod_sys_exit_group(int);

static void pid_tree_insert(int pid);
static void pid_tree_remove(int pid);
static int __init inotify_hook_init(void);
static void __exit inotify_hook_fini(void);


/*************************** PROCESS SYSCALL HOOK ***************************/
asmlinkage long mod_sys_execve(const char __user *filename, const char __user *const __user *argv, const char __user *const __user *envp)
{
    //TODO: insert pid node
    return ori_sys_execve(filename, argv, envp);
}
asmlinkage long mod_sys_fork(void)
{
    //TODO: insert pid node
    return ori_sys_fork();
}
asmlinkage long mod_sys_exit_group(int error_code)
{
    //TODO: remove pid node
    return ori_sys_exit_group(error_code);
}

/*************************** INOTIFY SYSCALL HOOK ***************************/
asmlinkage long mod_inotify_add_watch(int fd, const char __user *pathname, u32 mask)
{
    int wd;
    struct path path;
    unsigned int flags = 0;
    char *pname = NULL;
    char buf[PATH_MAX];
    pid_t usr_pid = task_pid_nr(current);

    wd = ori_inotify_add_watch(fd, pathname, mask);

    if (wd>=0 && user_path_at(AT_FDCWD, pathname, flags, &path)==0)
    {
        if (!(mask & IN_DONT_FOLLOW))
		    flags |= LOOKUP_FOLLOW;
        if (mask & IN_ONLYDIR)
            flags |= LOOKUP_DIRECTORY;
    
        pname = dentry_path_raw(path.dentry, buf, PATH_MAX);
        path_put(&path);

        //TODO: record_insert(usr_pid, fd, wd, pname);
        // printh("[%d] (%d %d + %s) \n", usr_pid, fd, wd, pname);
    }

    return wd;
}
asmlinkage long mod_inotify_rm_watch(int fd, __s32 wd)
{
    int ret;
    pid_t usr_pid = task_pid_nr(current);

    ret = ori_inotify_rm_watch(fd, wd);

    //TODO: record_remove(usr_pid, fd, wd);
    // printh("[%d] (%d %d - )\n", usr_pid, fd, wd);

    return ret;
}

/***************************** UTILITY FUNCTION *****************************/
static void pid_tree_insert(int pid)
{
    // struct *radix_tree_root fdwd_table;
    // kmem_cache_alloc(fdwd_table, GFP_NOFS);
    // INIT_RADIX_TREE(fdwd_table, GFP_ATOMIC);
}
static void pid_tree_remove(int pid)
{
    // struct *radix_tree_root fdwd_table;
    // kmem_cache_alloc(fdwd_table, GFP_NOFS);
    // INIT_RADIX_TREE(fdwd_table, GFP_ATOMIC);
}


/****************************** MAIN_ENTRY ******************************/
static int __init inotify_hook_init(void)
{
    /* get sys_call_table pointer */
    p_sys_call_table = (void **) kallsyms_lookup_name("sys_call_table");
    if (!p_sys_call_table)
    {
        printh("Cannot find sys_call_table address\n");
        return -EINVAL;
    }

    set_addr_rw((unsigned long)p_sys_call_table);
    ori_inotify_add_watch = p_sys_call_table[__NR_inotify_add_watch];
    ori_inotify_rm_watch  = p_sys_call_table[__NR_inotify_rm_watch];
    ori_sys_execve        = p_sys_call_table[__NR_sys_execve];
    ori_sys_fork          = p_sys_call_table[__NR_sys_fork];
    ori_sys_exit_group    = p_sys_call_table[__NR_sys_exit_group];
    p_sys_call_table[__NR_inotify_add_watch] = mod_inotify_add_watch;
    p_sys_call_table[__NR_inotify_rm_watch]  = mod_inotify_rm_watch;
    p_sys_call_table[__NR_sys_execve]        = mod_sys_execve;
    p_sys_call_table[__NR_sys_fork]          = mod_sys_fork;
    p_sys_call_table[__NR_sys_exit_group]    = mod_sys_exit_group;
    set_addr_ro((unsigned long)p_sys_call_table);

    printh("inotify module init.\n");
    return 0;
}

static void __exit inotify_hook_fini(void)
{
    set_addr_rw((unsigned long)p_sys_call_table);
    p_sys_call_table[__NR_inotify_add_watch] = ori_inotify_add_watch;
    p_sys_call_table[__NR_inotify_rm_watch]  = ori_inotify_rm_watch;
    p_sys_call_table[__NR_sys_execve]        = ori_sys_execve;
    p_sys_call_table[__NR_sys_fork]          = ori_sys_fork;
    p_sys_call_table[__NR_sys_exit_group]    = ori_sys_exit_group;
    set_addr_ro((unsigned long)p_sys_call_table);
    // printh("inotify module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");
