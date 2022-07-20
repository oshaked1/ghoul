enum service_calls {
    SERVICE_SHOW=0,
    SERVICE_UNLOAD,
};

int handle_ioctl_request(unsigned int fd, unsigned int cmd, unsigned long arg);