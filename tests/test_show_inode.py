import os
from time import sleep
import multiprocessing as mp

import ghoulctl


MODULE_NAME = os.environ.get('GHOUL_MODULE_NAME', 'ghoul')
FILE = '/tmp/hidden_file'


def can_access(path: str) -> bool:
    return os.access(path, os.F_OK)


def test_show_file_all_pids():
    # reload ghoul to make sure no inodes are hidden
    ghoulctl.unload()
    sleep(0.1)
    os.system(f'sudo insmod {MODULE_NAME}.ko')
    assert ghoulctl.ping()

    # create file which will be hidden
    open(FILE, 'w').close()

    # make sure file is accessible
    assert can_access(FILE)

    # hide file
    inode = os.stat(FILE).st_ino
    ghoulctl.hide_inode(inode)
    assert not can_access(FILE)

    # perform test
    ghoulctl.show_inode(inode, ghoulctl.ALL_PIDS)
    assert can_access(FILE)

    # delete the file
    os.remove(FILE)
    assert ghoulctl.ping()


def show_file_to_self(q: mp.Queue, inode: int):
    ghoulctl.show_inode(inode, os.getpid())
    q.put(can_access(FILE))


def test_show_file_specific_pid():
    # reload ghoul to make sure no inodes are hidden
    ghoulctl.unload()
    sleep(0.1)
    os.system(f'sudo insmod {MODULE_NAME}.ko')
    assert ghoulctl.ping()

    # create file which will be hidden
    open(FILE, 'w').close()

    # make sure file is accessible
    assert can_access(FILE)

    # hide file
    inode = os.stat(FILE).st_ino
    ghoulctl.hide_inode(inode)
    assert not can_access(FILE)

    # perform test
    q = mp.Queue()
    p = mp.Process(target=show_file_to_self, args=(q, inode))
    p.start()
    p.join()
    success = q.get()
    assert success
    assert not can_access(FILE) # make sure we still can't see the file

    # show file and delete it
    ghoulctl.show_inode(inode)
    os.remove(FILE)
    assert ghoulctl.ping()


def show_file_to_parent(q: mp.Queue, inode: int):
    ghoulctl.show_inode(inode, ghoulctl.PARENT_PID)

    # make sure it didn't affect us
    q.put(not can_access(FILE))


def test_show_file_parent_pid():
    # reload ghoul to make sure no inodes are hidden
    ghoulctl.unload()
    sleep(0.1)
    os.system(f'sudo insmod {MODULE_NAME}.ko')
    assert ghoulctl.ping()

    # create file which will be hidden
    open(FILE, 'w').close()

    # make sure file is accessible
    assert can_access(FILE)

    # hide file
    inode = os.stat(FILE).st_ino
    ghoulctl.hide_inode(inode)
    assert not can_access(FILE)

    # perform test
    q = mp.Queue()
    p = mp.Process(target=show_file_to_parent, args=(q, inode))
    p.start()
    p.join()
    success = q.get()
    assert success
    assert can_access(FILE)

    # delete the file
    os.remove(FILE)
    assert ghoulctl.ping()
