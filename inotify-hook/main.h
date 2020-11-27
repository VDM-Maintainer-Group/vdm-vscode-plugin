#ifndef __HOOK_MAIN_H__
#define __HOOK_MAIN_H__

#include <linux/slab.h>
#include <asm/tlbflush.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/namei.h>
#include <linux/string.h>
#include <linux/radix-tree.h>

#define IN_ONLYDIR		    0x01000000	/* only watch the path if it is a directory */
#define IN_DONT_FOLLOW		0x02000000	/* don't follow a sym link */
#define KERN_LOG KERN_NOTICE "[inotify_hook]"
#define printh(...) printk(KERN_LOG __VA_ARGS__)

struct radix_tree_root PID_TABLE;

void set_addr_rw(const unsigned long);
void set_addr_ro(const unsigned long);

void set_addr_rw(const unsigned long addr)
{
    unsigned int level;
    pte_t *pte;

    pte = lookup_address(addr, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW;

    local_flush_tlb();
}

void set_addr_ro(const unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte &~_PAGE_RW;
}

#endif