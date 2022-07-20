#! /bin/python3
import sys
import ctypes


syscall = ctypes.CDLL(None).syscall
SYSCALL_IOCTL_X86_64 = 16
SERVICE_FD = 666

SERVICE_SHOW = 0
SERVICE_UNLOAD = 1


def ioctl(fd: int, cmd: int, arg: int):
    syscall(SYSCALL_IOCTL_X86_64, fd, cmd, arg)


def show():
    ioctl(SERVICE_FD, SERVICE_SHOW, 0)


def unload():
    ioctl(SERVICE_FD, SERVICE_UNLOAD, 0)


def main():
    if len(sys.argv) < 2:
        print('ERROR: expected a command')
        exit()
    cmd = sys.argv[1]

    if cmd == 'show':
        show()
    elif cmd == 'unload':
        unload()
    else:
        print(f'unknown command {cmd}')


if __name__ == '__main__':
    main()
