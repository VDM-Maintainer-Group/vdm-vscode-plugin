/* Inotify WD Reverse Lookup Kernel Module
* References:
* 1. https://elixir.bootlin.com/linux/latest/source/include/linux/radix-tree.h
* 2. https://biscuitos.github.io/blog/IDA_ida_destroy/
* 3. https://lwn.net/Articles/175432/
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include "main.h"
#include "khook/khook/engine.c"
#include "netlink_comm.h"

/************************* PROTOTYPE DECLARATION *************************/
static struct comm_list_t comm_list;
static int __init inotify_hook_init(void);
static void __exit inotify_hook_fini(void);

/********************* COMM_RECORD UTILITY FUNCTION **********************/
static int comm_record_insert(struct comm_record_t *record, unsigned long pid, int fd, u32 wd, char * const pname)
{
    int ret;
    struct radix_tree_root *fd_wd_rt;

    fd_wd_rt = radix_tree_lookup(record->pid_rt, pid);
    if (!fd_wd_rt)
    {
        fd_wd_rt = kmalloc(sizeof(struct radix_tree_root), GFP_KERNEL);
        if (unlikely(!fd_wd_rt))
        {
            return -ENOMEM;
        }
        INIT_RADIX_TREE((*fd_wd_rt), GFP_ATOMIC);

        spin_lock(&record->lock);
        ret = radix_tree_insert(record->pid_rt, pid, fd_wd_rt)
        spin_unlock(&record->lock);

        if (unlikely(ret<0))
        {
            printh("comm_record: pid_rt allocation failed for %d.\n", pid);
            return ret;
        }
    }

    spin_lock(&record->lock);
    ret = radix_tree_insert(fd_wd_rt, fd_wd_to_mark(fd,wd), pname);
    spin_unlock(&record->lock);
    if (unlikely(ret<0))
    {
        printh("comm_record: fd_wd_rt allocation failed for (%d, %d).\n", fd, wd);
        return ret;
    }

    return 0;
}

static void comm_record_remove(struct comm_record_t *record, unsigned long pid, int fd, u32 wd)
{
    int mark;
    char *pathname;
    struct radix_tree_iter *iter, *fd_wd_rt;

    fd_wd_rt = (struct radix_tree_iter *) radix_tree_lookup(record->pid_rt, pid);
    if (!fd_wd_rt)
    {
        goto out;
    }

    mark = fd_wd_to_mark(fd,wd);
    pathname = (char *) radix_tree_lookup(fd_wd_rt, mark);
    if (!pathname)
    {
        goto out;
    }

    spin_lock(&record->lock);
    {
        // free record fd_wd_rt
        kfree(pathname);
        radix_tree_delete(fd_wd_rt, mark);
        // free record of pid_rt
        if (radix_tree_empty(fd_wd_rt))
        {
            kfree(fd_wd_rt)
            radix_tree_delete(record->pid_rt, pid);
        }
    }
    spin_unlock(&record->lock);

out:
    return;
}

//TODO: radix tree dump for print

static void comm_record_init(struct comm_record_t *record)
{
    spin_lock_init(&record->lock);
    INIT_RADIX_TREE(record->pid_rt, GFP_ATOMIC);
    return;
}

static void comm_record_cleanup(struct comm_record_t *record)
{
    struct radix_tree_iter iter0, iter1;
	void **slot0, **slot1;

    spin_lock(&record->lock);
    radix_tree_for_each_slot(slot0, &record->pid_rt, &iter0, 0)
    {
        struct radix_tree_root *p_fd_wd_rt = radix_tree_deref_slot(slot0);
        if (!radix_tree_exception(fd_wd_rt))
        {
            radix_tree_for_each_slot(slot1, p_fd_wd_rt, &iter1, 0)
            {
                void *pathname = radix_tree_deref_slot(slot1);
                if (!radix_tree_exception(pathname)) {kfree(pathname);}
                radix_tree_iter_delete(p_fd_wd_rt, &iter1, slot1);
            }
        }
        radix_tree_iter_delete(&record->pid_rt, &iter0, slot0);
    }
    spin_unlock(&record->lock);
    
    return;
}

/********************** COMM_LIST UTILITY FUNCTION ***********************/
struct comm_list_item * comm_list_find(const char *name)
{
    struct comm_list_item *item=NULL, *result=NULL;

    spin_lock(&comm_list.lock);
    list_for_each_entry(item, &comm_list.head, node)
    {
        if (strcmp(name, item->comm_name)==0)
        {
            result = item;
            break;
        }
    }
    spin_unlock(&comm_list.lock);

    return result;
}

static int comm_list_add(const char *name)
{
    struct comm_list_item *item;
    // check duplicated allocations
    if (unlikely(comm_list_find(name)))
    {
        goto out;
    }
    // allocate memory
    item = kmalloc(sizeof(struct comm_list_item), GFP_ATOMIC);
    if (unlikely(!item))
    {
        return -ENOMEM;
    }
    item->comm_name = kmalloc(strlen(name), GFP_ATOMIC);
    if (unlikely(!item->comm_name))
    {
        return -ENOMEM;
    }
    // initialize the item
    strcpy(item->comm_name, name);
    comm_record_init(&item->record);
    INIT_LIST_HEAD(&item->node);
    // add onto comm_list
    spin_lock(&comm_list.lock);
    list_add(&item->node, &comm_list.head);
    spin_unlock(&comm_list.lock);
    printh("comm_list add \"%s\"\n", name);
out:
    return 0;
}

static void comm_list_rm(struct comm_list_item *item)
{
    assert_spin_locked(&comm_list->lock);

    comm_record_cleanup(&item->record);
    kfree(item->comm_name);
    list_del(&item->node);
    kfree(item);

    return;
}

static void comm_list_rm_by_name(const char *name)
{
    struct comm_list_item *item=NULL, *tmp=NULL;

    spin_lock(&comm_list.lock);
    list_for_each_entry_safe(item, tmp, &comm_list.head, node)
    {
        if (strcmp(name, item->comm_name)==0)
        {
            comm_list_rm(item);
            break;
        }
    }
    spin_unlock(&comm_list.lock);

    return;
}

static int comm_list_init(void)
{
    int ret = 0;

    spin_lock_init(&comm_list.lock);
    INIT_LIST_HEAD(&comm_list.head);
    ret = comm_list_add("code"); //to be removed
    
    return ret;
}

static void comm_list_exit(void)
{
    struct comm_list_item *item=NULL, *tmp=NULL;

    spin_lock(&comm_list.lock);
    list_for_each_entry_safe(item, tmp, &comm_list.head, node)
    {
        comm_list_rm(item);
    }
    spin_unlock(&comm_list.lock);

    return;
}

/*************************** INOTIFY SYSCALL HOOK ***************************/
//regs->(di, si, dx, r10), reference: arch/x86/include/asm/syscall_wrapper.h#L125
//SYSCALL_DEFINE3(inotify_add_watch, int, fd, const char __user *, pathname, u32, mask)
KHOOK_EXT(long, ORIGIN(inotify_add_watch), const struct pt_regs *);
static long MODIFY(inotify_add_watch)(const struct pt_regs *regs)
{
    int wd;
    struct path path;
    unsigned int flags = 0;
    char buf[PATH_MAX];
    char *pname=NULL, *precord=NULL;
    struct comm_list_item *item;
    // decode the registers
    int fd = (int) regs->di;
    const char __user *pathname = (char __user *) regs->si;
    u32 mask = (u32) regs->dx;

    // do the original function
    wd = KHOOK_ORIGIN(ORIGIN(inotify_add_watch), regs);
    // get the pathname
    if (!(mask & IN_DONT_FOLLOW))
        flags |= LOOKUP_FOLLOW;
    if (mask & IN_ONLYDIR)
        flags |= LOOKUP_DIRECTORY;
    if ( wd>=0 && (item=comm_list_find(current->comm)) && (user_path_at(AT_FDCWD, pathname, flags, &path)==0) )
    {
        pname = dentry_path_raw(path.dentry, buf, PATH_MAX);
        path_put(&path);
        // insert into comm_record
        precord = kmalloc(strlen(pname), GFP_ATOMIC);
        if (unlikely(!precord))
        {
            return -ENOMEM;
        }
        strcpy(precord, pname); //"pname" points to "buf[PATH_MAX]"
        comm_record_insert(item, task_pid_nr(current), fd, wd, precord);
        printh("%s, PID %d add (%d,%d): %s\n", current->comm, task_pid_nr(current), fd, wd, precord);
    }
    return wd;
}

//SYSCALL_DEFINE2(inotify_rm_watch, int, fd, __s32, wd)
KHOOK_EXT(long, ORIGIN(inotify_rm_watch), const struct pt_regs *regs);
static long MODIFY(inotify_rm_watch)(const struct pt_regs *regs)
{
    int ret;
    struct comm_list_item *item;
    // decode the registers
    int fd = (int) regs->di;
    u32 wd = (u32) regs->si;

    ret = KHOOK_ORIGIN(ORIGIN(inotify_rm_watch), regs);

    if ((item=comm_list_find(current->comm)))
    {
        // remove from comm_record
        comm_record_remove(item, task_pid_nr(current), fd, wd);
        printh("%s, PID %d remove (%d,%d)\n", current->comm, task_pid_nr(current), fd, wd);
    }

    return ret;
}

/****************************** MAIN_ENTRY ******************************/
static int __init inotify_hook_init(void)
{
    int ret = 0;

    /* init data structure */
    if ( (ret=comm_list_init()) < 0 )
    {
        return ret;
    }

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
    khook_cleanup();
    comm_list_exit();
    printh("Inotify hook module exit.\n\n");
}

module_init(inotify_hook_init);
module_exit(inotify_hook_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VDM");
