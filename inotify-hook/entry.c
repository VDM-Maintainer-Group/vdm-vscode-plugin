#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/kallsyms.h>
#include <asm/tlbflush.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>

#define KERN_LOG KERN_NOTICE "[printk]"
#define printh(...) printk(KERN_LOG __VA_ARGS__)

void **p_sys_call_table;

/*
static void set_addr_rw(const unsigned long addr)
{
    unsigned int level;
    pte_t *pte = lookup_address(addr, &level);
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
*/

asmlinkage int (*old_inotify_add_watch) (int, const char __user *, u32);
asmlinkage int new_inotify_add_watch(int fd, const char __user *pathname, u32 mask)
{
/*
    pid_t usr_pid = current->pid;
    char realpath[128];

    //FIXME: could not read the pathname
    if (strncpy_from_user(realpath, pathname, 128) != 0)
    {
        printh("could could get real pathname.");
        return -EFAULT;
    }
    
    printh("%d add watch on %s\n", usr_pid, realpath);
*/
    printh("I hooked, and let go.");
    return ori_inotify_add_watch(fd, pathname, mask);
}

static int __init inotify_hook_init(void)
{
    write_cr0(read_cr0() & (~0x10000));

    p_sys_call_table = (void **) kallsyms_lookup_name("sys_call_table");
    //set_addr_rw((unsigned long)p_sys_call_table);
    ori_inotify_add_watch = p_sys_call_table[__NR_inotify_add_watch];
    p_sys_call_table[__NR_inotify_add_watch] = mod_inotify_add_watch;

    write_cr0(read_cr0() | (0x10000));

    return 0;
}

static void __exit inotify_hook_fini(void)
{
    //p_sys_call_table[__NR_inotify_add_watch] = ori_inotify_add_watch;
    write_cr0(read_cr0() & (~0x10000));
    set_addr_ro((unsigned long) p_sys_call_table);
    write_cr0(read_cr0() | (0x10000));
    printh("inotify module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");
