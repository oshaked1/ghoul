import os
import multiprocessing as mp

import ghoulctl


def give_ourselves_root(q: mp.Queue):
    ghoulctl.give_root(ghoulctl.THIS_PID)
    q.put(os.getuid() == 0 and os.geteuid() == 0)


def test_give_us_root():
    # make sure conditions are right
    assert ghoulctl.ping()
    assert os.getuid() != 0 and os.geteuid() != 0

    # perform test
    q = mp.Queue()
    p = mp.Process(target=give_ourselves_root, args=(q,))
    p.start()
    p.join()
    success = q.get()
    assert success
    assert ghoulctl.ping()


def give_parent_root_child():
    ghoulctl.give_root(ghoulctl.PARENT_PID)


def give_parent_root_parent(q: mp.Queue):
    p = mp.Process(target=give_parent_root_child)
    p.start()
    p.join()

    q.put(os.getuid() == 0 and os.geteuid() == 0)


def test_give_parent_root():
    # make sure conditions are right
    assert ghoulctl.ping()
    assert os.getuid() != 0 and os.geteuid() != 0

    # perform test
    q = mp.Queue()
    p = mp.Process(target=give_parent_root_parent, args=(q,))
    p.start()
    p.join()
    success = q.get()
    assert success
    assert ghoulctl.ping()


def give_other_root_giver(q: mp.Queue):
    pid = q.get(timeout=2)
    ghoulctl.give_root(pid)
    q.put(True)


def give_other_root_receiver(q1: mp.Queue, q2: mp.Queue):
    q1.put(os.getpid())
    q1.get(timeout=2)

    q2.put(os.getuid() == 0 and os.geteuid() == 0)


def test_give_other_root():
    # make sure conditions are right
    assert ghoulctl.ping()
    assert os.getuid() != 0 and os.geteuid() != 0

    # perform test
    q1 = mp.Queue()
    q2 = mp.Queue()
    p1 = mp.Process(target=give_other_root_giver, args=(q1,))
    p2 = mp.Process(target=give_other_root_receiver, args=(q1, q2))
    p1.start()
    p2.start()
    p1.join()
    p2.join()

    success = q2.get()
    assert success
    assert ghoulctl.ping()
