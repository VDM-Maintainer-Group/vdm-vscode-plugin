#ifndef __HOOK_MAIN_H__
#define __HOOK_MAIN_H__

#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/namei.h>
#include <linux/string.h>
#include <linux/types.h>
//headers for utility functions
#include <linux/list.h>
#include <linux/radix-tree.h>
#include <linux/spinlock.h>

#define IN_ONLYDIR		    0x01000000	/* only watch the path if it is a directory */
#define IN_DONT_FOLLOW		0x02000000	/* don't follow a sym link */
#define KERN_LOG KERN_NOTICE "[inotify_hook]"
#define printh(...) printk(KERN_LOG __VA_ARGS__)

#ifdef CONFIG_X86_64
#define ORIGIN(FUNC) __x64_sys_##FUNC
#define MODIFY(FUNC) khook___x64_sys_##FUNC
#else
#error Target CPU architecture is NOT supported !!!
#endif

struct comm_list_t
{
    u32 counter; //not used for now
    spinlock_t lock;
    struct list_head head;
};

struct comm_record_t
{
    spinlock_t lock;
    struct radix_tree_root pid_rt; //pid->(wd+fd*MAX_NUM_WATCH)->pathname
};

struct comm_list_item
{
    char *comm_name;
    struct comm_record_t record;
    struct list_head node;
};

static inline unsigned long fd_wd_to_mark(u32 fd, u32 wd)
{
    return wd*1000+fd;
}

#endif