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

/********************** COMM_LIST UTILITY FUNCTION ***********************/
static int comm_list_find(const char *name)
{
    int ret = 0;
    struct comm_list_item *item;

    spin_lock(&comm_list.comm_lock);
    list_for_each_entry(item, &comm_list.head, node)
    {
        if (strcmp(name, item->name)==0)
        {
            ret = 1;
            break;
        }
    }
    spin_unlock(&comm_list.comm_lock);

    return ret;
}

static int comm_list_add(const char *name)
{
    struct comm_list_item *item;

    if (comm_list_find(name))
    {
        goto out;
    }

    item = kmalloc(sizeof(struct comm_list_item), GFP_ATOMIC);
    if (unlikely(!item))
    {
        return -ENOMEM;
    }
    item->name = kmalloc(strlen(name), GFP_ATOMIC);
    if (unlikely(!item->name))
    {
        return -ENOMEM;
    }

    strcpy(item->name, name);
    INIT_LIST_HEAD(&item->node);

    spin_lock(&comm_list.comm_lock);
    list_add(&item->node, &comm_list.head);
    spin_unlock(&comm_list.comm_lock);
    printh("comm_list add \"%s\"\n", name);
out:
    return 0;
}

static void comm_list_rm(const char *name)
{
    struct comm_list_item *item=NULL, *tmp=NULL;

    spin_lock(&comm_list.comm_lock);
    list_for_each_entry_safe(item, tmp, &comm_list.head, node)
    {
        if (strcmp(name, item->name)==0)
        {
            kfree(item->name);
            list_del_init(&item->node);
            kfree(item);
            break;
        }
    }
    spin_unlock(&comm_list.comm_lock);

    return;
}

static int comm_list_init(void)
{
    int ret = 0;

    spin_lock_init(&comm_list.comm_lock);
    INIT_LIST_HEAD(&comm_list.head);
    ret = comm_list_add("code"); //to be removed
    
    return ret;
}

static void comm_list_exit(void)
{
    struct comm_list_item *item=NULL, *tmp=NULL;

    spin_lock(&comm_list.comm_lock);
    list_for_each_entry_safe(item, tmp, &comm_list.head, node)
    {
        kfree(item->name);
        list_del_init(&item->node);
        kfree(item);
    }
    spin_unlock(&comm_list.comm_lock);

    return;
}

/********************** WATCHES_RT UTILITY FUNCTION ***********************/

/*************************** INOTIFY SYSCALL HOOK ***************************/
//regs->(di, si, dx, r10), reference: arch/x86/include/asm/syscall_wrapper.h#L125
//SYSCALL_DEFINE3(inotify_add_watch, int, fd, const char __user *, pathname, u32, mask)
KHOOK_EXT(long, ORIGIN(inotify_add_watch), const struct pt_regs *);
static long MODIFY(inotify_add_watch)(const struct pt_regs *regs)
{
    int wd;
    struct path path;
    unsigned int flags = 0;
    char *pname = NULL;
    char buf[PATH_MAX];
    unsigned int usr_pid = task_pid_nr(current); //current->pid;
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
    if ( comm_list_find(current->comm) && (user_path_at(AT_FDCWD, pathname, flags, &path)==0) )
    {
        pname = dentry_path_raw(path.dentry, buf, PATH_MAX);
        path_put(&path);
        printh("%s, PID %d add (%d,%d): %s\n", current->comm, usr_pid, fd, wd, pname);

        //TODO: insert into wathes_rt, if comm_name in comm_list
    }
    return wd;
}

//SYSCALL_DEFINE2(inotify_rm_watch, int, fd, __s32, wd)
KHOOK_EXT(long, ORIGIN(inotify_rm_watch), const struct pt_regs *regs);
static long MODIFY(inotify_rm_watch)(const struct pt_regs *regs)
{
    int ret;
    // decode the registers
    int fd = (int) regs->di;
    u32 wd = (u32) regs->si;

    ret = KHOOK_ORIGIN(ORIGIN(inotify_rm_watch), regs);

    if (comm_list_find(current->comm))
    {
        printh("%s, PID %d remove (%d,%d)\n", current->comm, task_pid_nr(current), fd, wd);

        //TODO: remove from wathes_rt, if comm_name in comm_list
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
