#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include "main.h"


/************************* PROTOTYPE DECLARATION *************************/
void **p_sys_call_table;
asmlinkage int (*ori_inotify_add_watch) (int, const char __user *, u32);
asmlinkage int mod_inotify_add_watch(int, const char __user *, u32);
asmlinkage int (*ori_inotify_rm_watch) (int, __s32);
asmlinkage int mod_inotify_rm_watch(int, __s32);
static int __init inotify_hook_init(void);
static void __exit inotify_hook_fini(void);

/*************************** INOTIFY_ADD_WATCH ***************************/
asmlinkage int mod_inotify_add_watch(int fd, const char __user *pathname, u32 mask)
{
    int wd;
    struct path path;
    unsigned int flags = 0;
    char *pname = NULL;
    char buf[PATH_MAX];
    pid_t usr_pid = task_pid_nr(current);

    wd = ori_inotify_add_watch(fd, pathname, mask);

    if (!(mask & IN_DONT_FOLLOW))
		flags |= LOOKUP_FOLLOW;
	if (mask & IN_ONLYDIR)
		flags |= LOOKUP_DIRECTORY;
    
    if (wd>0 && user_path_at(AT_FDCWD, pathname, flags, &path)==0)
    {
        pname = dentry_path_raw(path.dentry, buf, PATH_MAX);
        path_put(&path);
        printh("[%d] (%d %d + %s) \n", usr_pid, fd, wd, pname);
    }

    return wd;
}
/*************************** INOTIFY_RM_WATCH ***************************/
asmlinkage int mod_inotify_rm_watch(int fd, __s32 wd)
{
    int ret;
    pid_t usr_pid = task_pid_nr(current);

    ret = ori_inotify_rm_watch(fd, wd);

    printh("[%d] (%d %d - )\n", usr_pid, fd, wd);

    return ret;
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

    /* syscall replacement */
    set_addr_rw((unsigned long)p_sys_call_table);
    ori_inotify_add_watch = p_sys_call_table[__NR_inotify_add_watch];
    p_sys_call_table[__NR_inotify_add_watch] = mod_inotify_add_watch;
    ori_inotify_rm_watch = p_sys_call_table[__NR_inotify_rm_watch];
    p_sys_call_table[__NR_inotify_rm_watch] = mod_inotify_rm_watch;
    set_addr_ro((unsigned long)p_sys_call_table);

    printh("inotify module init.\n");
    return 0;
}

static void __exit inotify_hook_fini(void)
{
    set_addr_rw((unsigned long)p_sys_call_table);
    p_sys_call_table[__NR_inotify_add_watch] = ori_inotify_add_watch;
    p_sys_call_table[__NR_inotify_rm_watch]  = ori_inotify_rm_watch;
    set_addr_ro((unsigned long)p_sys_call_table);
    printh("inotify module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");
