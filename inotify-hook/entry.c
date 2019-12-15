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
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/utsname.h>

#define KERN_LOG KERN_NOTICE "[printk]"
#define printh(...) printk(KERN_LOG __VA_ARGS__)

#define CR0_WP 0x00010000   // Write-Protect Bit (CR0:16) amd64
#ifndef _LP64
#error "Only supports x86_64 kernel <=> cpu!"
#endif

void **sys_call_table;
/*************************** STRICT PROTOTYPES ***************************/
asmlinkage long (*ori_inotify_add_watch) (int, const char __user *, u32);
asmlinkage long mod_inotify_add_watch(int, const char __user *, u32);
unsigned long **find_sys_call_table(void);
static int __init inotify_hook_init(void);
static void __exit inotify_hook_fini(void);
/*Kernel >4.1 no longer exports set_memory_r*, here it's a fix :)*/
static int (*set_memory_rw)(unsigned long addr, int numpages) = NULL;
static int (*set_memory_ro)(unsigned long addr, int numpages) = NULL;

/*************************** INOTIFY_ADD_WATCH ***************************/
asmlinkage long mod_inotify_add_watch(int fd, const char __user *pathname, u32 mask)
{
    pid_t usr_pid = current->pid;
    char realpath[128];

    //FIXME: could not read the pathname
    if (strncpy_from_user(realpath, pathname, 128) != 0)
    {
        printh("could could get real pathname.\n");
        return -EFAULT;
    }
    
    printh("%d add watch on %s\n", usr_pid, realpath);
    return ori_inotify_add_watch(fd, pathname, mask);
}

/*************************** INOTIFY_RM_WATCH ***************************/
// asmlinkage int (*ori_inotify_rm_watch) (int, __s32);
// asmlinkage int mod_inotify_rm_watch(int fd, __s32 wd)
// {
//     return ori_inotify_rm_watch(fd, wd)
// }

/************************* ALTERNATIVE FUNCTION *************************/
unsigned long **find_sys_call_table()
{
    unsigned long ptr;
    unsigned long *p;
    static long (*sys_close) (unsigned int fd)=NULL;

    sys_close=(void *)kallsyms_lookup_name("sys_close");
    if (!sys_close)
    {
        printk(KERN_DEBUG "[HOOK] Symbol sys_close not found\n");
        return NULL;
    }

    /* the sys_call_table can be found between the addresses of sys_close
     * and loops_pre_jiffy. Look at /boot/System.map or /proc/kallsyms to
     * see if it is the case for your kernel */
    for (ptr = (unsigned long)sys_close;
            ptr < (unsigned long)&loops_per_jiffy;
            ptr += sizeof(void *))
    {
        p = (unsigned long *)ptr;
        /* Indexes such as __NR_close can be found in
         * /usr/include/x86_64-linux-gnu/asm/unistd{,_64}.h
         * syscalls function can be found in
         * /usr/src/`uname -r`/include/linux/syscalls.h */
        if (p[__NR_close] == (unsigned long)sys_close)
        {
            /* the address of the ksys_close function is equal to the one found
             * in the sys_call_table */
            printk(KERN_DEBUG "[HOOK] Found the sys_call_table!!!\n");
            return (unsigned long **)p;
        }
    }
    return NULL;
}

/****************************** MAIN_ENTRY ******************************/
static int __init inotify_hook_init(void)
{
    int ret;
    unsigned long cr0;

    /* get sys_call_table pointer */
    sys_call_table = (void **) find_sys_call_table();
    if (!sys_call_table)
    {
        sys_call_table = (void **) kallsyms_lookup_name("sys_call_table");
        if (!sys_call_table)
        {
            printh("Cannot find sys_call_table address\n");
            return -EINVAL;
        }
    }
    printh("Find sys_call_table at 0x%16lx\n", (unsigned long)sys_call_table);

    /* get set_memory_rw & set_memory_ro operation */
    cr0 = read_cr0();
    write_cr0(cr0 & (~CR0_WP)); //disable the write-protect bit
    set_memory_rw = (void *)kallsyms_lookup_name("set_memory_rw");
    set_memory_ro = (void *)kallsyms_lookup_name("set_memory_ro");
    if (set_memory_rw==NULL || set_memory_ro==NULL)
    {
        printh("set_memory operation not found.\n");
        return -EINVAL;
    }

    /* set syscall table r/w and hook */
    ret = set_memory_rw(PAGE_ALIGN((unsigned long)sys_call_table),1);
    if (ret)
    {
        printh("Cannot set the memory to rw (%d)\n", ret);
        return -EINVAL;
    }
    ori_inotify_add_watch = sys_call_table[__NR_inotify_add_watch];
    sys_call_table[__NR_inotify_add_watch] = mod_inotify_add_watch;
    printh("Replace inotify_add_watch (0x%16lx) with modified (0x%16lx)\n", \
            (unsigned long)ori_inotify_add_watch, (unsigned long)mod_inotify_add_watch);
    write_cr0(cr0);

    return 0;
}

static void __exit inotify_hook_fini(void)
{
    unsigned long cr0;
    cr0 = read_cr0();
    write_cr0(cr0 & ~CR0_WP);
    
    sys_call_table[__NR_inotify_add_watch] = ori_inotify_add_watch;
    set_memory_ro(PAGE_ALIGN((unsigned long)sys_call_table), 1);

    write_cr0(cr0);
    printh("inotify module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");
