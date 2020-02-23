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
#define KERN_LOG KERN_NOTICE "[printk]"
#define printh(...) printk(KERN_LOG __VA_ARGS__)

/************************* PROTOTYPE DECLARATION *************************/
void **p_sys_call_table;
static void set_addr_rw(const unsigned long);
static void set_addr_ro(const unsigned long);
asmlinkage long (*ori_inotify_add_watch) (int, const char __user *, u32);
asmlinkage long mod_inotify_add_watch(int, const char __user *, u32);
static int __init inotify_hook_init(void);
static void __exit inotify_hook_fini(void);

static void set_addr_rw(const unsigned long addr)
{
    unsigned int level;
    pte_t *pte;

    pte = lookup_address(addr, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW;

    local_flush_tlb();
}

static void set_addr_ro(const unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte &~_PAGE_RW;
}

/*************************** INOTIFY_ADD_WATCH ***************************/
asmlinkage long mod_inotify_add_watch(int fd, const char __user *pathname, u32 mask)
{
    long ret;
    pid_t usr_pid = task_pid_nr(current);
    struct audit_context *context = current->audit_context;

    printh("%d, 0x%16lx is the context.\n", usr_pid, (unsigned long)context);
    ret = ori_inotify_add_watch(fd, pathname, mask);
/*
    list_for_each_entry(n, &context->names_list, list) {
        if (!n->name)
            continue;
        if (n->nmae->uptr == pathname)
        {
            kname = n->name->iname;
        }
    }

    if (kname)
    {
        printh("%d add watch on %s\n", usr_pid, kname);   
    }
    else
    {
        printh("(%ld) 0x%16lx failed to match existing pathname.\n", ret, (unsigned long)pathname);
    }
*/
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
    else
    {
        printh("Find sys_call_table at 0x%16lx\n", (unsigned long)p_sys_call_table);
    }

    /* syscall replacement */
    set_addr_rw((unsigned long)p_sys_call_table);
    ori_inotify_add_watch = p_sys_call_table[__NR_inotify_add_watch];
    p_sys_call_table[__NR_inotify_add_watch] = mod_inotify_add_watch;
    set_addr_ro((unsigned long)p_sys_call_table);
    printh("Replace inotify_add_watch (0x%16lx) with modified (0x%16lx)\n", \
            (unsigned long)ori_inotify_add_watch, (unsigned long)mod_inotify_add_watch);

    return 0;
}

static void __exit inotify_hook_fini(void)
{
    set_addr_rw((unsigned long)p_sys_call_table);
    p_sys_call_table[__NR_inotify_add_watch] = ori_inotify_add_watch;
    set_addr_ro((unsigned long)p_sys_call_table);
    printh("inotify module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");
