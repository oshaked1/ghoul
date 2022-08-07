import os
import subprocess
import multiprocessing as mp
from time import sleep

import ghoulctl


MODULE_NAME = os.environ.get('GHOUL_MODULE_NAME', 'ghoul')
FILE = '/tmp/hidden_file'
LINK = '/tmp/hidden_file_link'
DIR = '/tmp/hidden_dir'
TEST_DIR = '/tmp/test_dir'


def can_setxattr(path: str) -> bool:
    try:
        os.setxattr(path, 'user.dummy_attr', b'dummy_value')
    except FileNotFoundError:
        return False
    else:
        return True


def can_getxattr(path: str) -> bool:
    try:
        os.getxattr(path, 'user.dummy_attr')
    except FileNotFoundError:
        return False
    # no data available
    except OSError:
        return True
    else:
        return True


def can_listxattr(path: str) -> bool:
    try:
        os.listxattr(path)
    except FileNotFoundError:
        return False
    else:
        return True


def can_removexattr(path: str) -> bool:
    try:
        os.removexattr(path, 'user.dummy_attr')
    except FileNotFoundError:
        return False
    # no data available
    except OSError:
        return True
    else:
        return True


def can_unlink(path: str) -> bool:
    try:
        os.unlink(path)
    except FileNotFoundError:
        return False
    else:
        return True


def can_symlink(file: str, link: str) -> bool:
    # create link
    os.symlink(file, link)

    # check if we can access the link
    res = os.access(link, os.F_OK, follow_symlinks=True)

    # remove link
    os.remove(link)

    return res


def can_link(file: str, link: str) -> bool:
    try:
        os.link(file, link)
    except FileNotFoundError:
        return False
    else:
        os.remove(link)
        return True


def can_rename(path: str) -> bool:
    try:
        os.rename(path, f'{path}_renamed')
    except FileNotFoundError:
        return False
    else:
        os.rename(f'{path}_renamed', path)
        return True


def can_truncate(path: str) -> bool:
    try:
        os.truncate(path, 0)
    except FileNotFoundError:
        return False
    else:
        return True


def can_access(path: str) -> bool:
    return os.access(path, os.F_OK)


def can_chmod(path: str) -> bool:
    try:
        os.chmod(path, 0o777)
    except FileNotFoundError:
        return False
    else:
        return True


def can_chown(path: str) -> bool:
    try:
        os.chown(path, os.getuid(), os.getgid())
    except FileNotFoundError:
        return False
    else:
        return True


def can_open(path: str) -> bool:
    try:
        open(path, 'r').close()
    except FileNotFoundError:
        return False
    else:
        return True


def can_readlink(path: str) -> bool:
    try:
        os.readlink(path)
    except FileNotFoundError:
        return False
    # not a link
    except OSError:
        return True
    else:
        return True


def can_stat(path: str) -> bool:
    try:
        os.stat(path)
    except FileNotFoundError:
        return False
    else:
        return True


def can_exec(path: str) -> bool:
    try:
        subprocess.check_call(path)
    except FileNotFoundError:
        return False
    else:
        return True


def can_umount(path: str) -> bool:
    proc = subprocess.Popen(['umount', path], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return 'No such file or directory' not in proc.communicate()[1].decode()


def can_mount(source: str, target: str) -> bool:
    proc = subprocess.Popen(['sudo', 'mount' , '-B', source, target], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    res = 'mount point does not exist' not in proc.communicate()[1].decode()

    # unmount in case it worked
    os.system(f'sudo umount {target}')

    return res


def can_pivot_root(path: str):
    # TODO: implement this (requires moving to a new mount namespace)
    return False


def can_statfs(path: str):
    try:
        os.statvfs(path)
    except FileNotFoundError:
        return False
    else:
        return True


def chdir(path: str, q: mp.Queue):
    try:
        os.chdir(path)
    except FileNotFoundError:
        q.put(False)
    else:
        q.put(True)


def can_chdir(path: str):
    q = mp.Queue()
    p = mp.Process(target=chdir, args=(path, q))
    p.start()
    p.join()
    return q.get()


def chroot(path: str, q: mp.Queue):
    try:
        os.chroot(path)
    except FileNotFoundError:
        q.put(False)
    except Exception:
        q.put(True)
    else:
        q.put(True)


def can_chroot(path: str):
    q = mp.Queue()
    p = mp.Process(target=chroot, args=(path, q))
    p.start()
    p.join()
    return q.get()


def test_hide_file_direct_access():
    # reload ghoul to make sure no inodes are hidden
    ghoulctl.unload()
    sleep(0.1)
    os.system(f'sudo insmod {MODULE_NAME}.ko')
    assert ghoulctl.ping()

    # create executable file which will be hidden
    with open(FILE, 'w') as f:
        f.write("#! /bin/python3\nexit(0)")
    os.chmod(FILE, 0o777)

    # make sure file is accessible
    assert can_access(FILE)

    # hide file
    inode = os.stat(FILE).st_ino
    ghoulctl.hide_inode(inode)

    # make sure file is not accessible anymore
    assert not can_setxattr(FILE)
    assert not can_getxattr(FILE)
    assert not can_listxattr(FILE)
    assert not can_removexattr(FILE)
    assert not can_unlink(FILE)
    assert not can_symlink(FILE, LINK)
    assert not can_link(FILE, LINK)
    assert not can_rename(FILE)
    assert not can_truncate(FILE)
    assert not can_access(FILE)
    assert not can_chmod(FILE)
    assert not can_chown(FILE)
    assert not can_open(FILE)
    assert not can_readlink(FILE)
    assert not can_stat(FILE)
    assert not can_exec(FILE)

    # show file and delete it
    ghoulctl.show_inode(inode)
    os.remove(FILE)
    assert ghoulctl.ping()


def test_hide_dir_direct_access():
    # reload ghoul to make sure no inodes are hidden
    ghoulctl.unload()
    sleep(0.1)
    os.system(f'sudo insmod {MODULE_NAME}.ko')
    assert ghoulctl.ping()

    # create directory which will be hidden
    os.makedirs(DIR, exist_ok=True)

    # make sure directory is accessible
    assert can_access(DIR)

    # hide dir
    inode = os.stat(DIR).st_ino
    ghoulctl.hide_inode(inode)

    # make sure dir is not accessible anymore
    assert not can_access(DIR)
    assert not can_open(DIR)

    assert not can_umount(DIR)

    os.makedirs(TEST_DIR, exist_ok=True)
    assert not can_mount(TEST_DIR, DIR)
    try:
        os.rmdir(TEST_DIR)
    except FileNotFoundError:
        pass

    assert not can_pivot_root(DIR)
    assert not can_statfs(DIR)
    assert not can_chdir(DIR)
    assert not can_chroot(DIR)

    # new syscalls, need to add tests for these
    #assert not can_open_tree(DIR)
    #assert not can_move_mount(DIR)
    #assert not can_fsopen(DIR)
    #assert not can_fspick(DIR)
    
    # show directory and delete it
    ghoulctl.show_inode(inode)
    os.rmdir(DIR)
    assert ghoulctl.ping()
