#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/kallsyms.h>
#include <asm/tlbflush.h>
#include <linux/sched.h>
#include <linux/uaccess.h>


#define KERN_LOG KERN_NOTICE "[printk]"
#define printh(...) printk(KERN_LOG __VA_ARGS__)

void **p_sys_call_table;

asmlinkage int (*ori_inotify_add_watch) (int, const char __user *, u32);
asmlinkage int mod_inotify_add_watch(int fd, const char __user *pathname, u32 mask)
{
    pid_t usr_pid = current->pid;
    char realpath[128];

    //FIXME: could not read the pathname
    if (strncpy_from_user(realpath, pathname, 128) != 0)
    {
        printh("could could get real pathname.");
        return -EFAULT;
    }
    
    printh("%d add watch on %s\n", usr_pid, realpath);
    return ori_inotify_add_watch(fd, pathname, mask);
}

// asmlinkage int (*ori_inotify_rm_watch) (int, __s32);
// asmlinkage int mod_inotify_add_watch(int fd, __s32 wd)
// {
//     return ori_inotify_rm_watch(fd, wd)
// }

static void set_addr_rw(const unsigned long addr)
{
    unsigned int level;
    pte_t *pte;

    pte = lookup_address(addr, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW;
    local_flush_tlb();
}

int set_addr_ro(unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte &~_PAGE_RW;
    return 0;
}

static int __init inotify_hook_init(void)
{
    p_sys_call_table = (void *) kallsyms_lookup_name("sys_call_table");
    ori_inotify_add_watch = p_sys_call_table[__NR_inotify_add_watch];

    set_addr_rw((unsigned long)p_sys_call_table);
    p_sys_call_table[__NR_inotify_add_watch] = mod_inotify_add_watch;

    return 0;
}

static void __exit inotify_hook_fini(void)
{
    p_sys_call_table[__NR_inotify_add_watch] = ori_inotify_add_watch;
    set_addr_ro((unsigned long) p_sys_call_table);
    printh("inotify module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");