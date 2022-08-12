import os

import ghoulctl
from utils import output


MODULE_NAME = os.environ.get('GHOUL_MODULE_NAME', 'ghoul')


def is_hidden_procfs():
    return MODULE_NAME not in output('lsmod')


def is_hidden_sysfs():
    return MODULE_NAME not in os.listdir('/sys/module')


def is_hidden():
    return is_hidden_procfs() and is_hidden_sysfs()


def test_show_and_hide():
    # make sure conditions are right
    assert ghoulctl.ping()
    assert is_hidden()
    
    # perform test
    ghoulctl.show()
    assert not is_hidden()

    ghoulctl.hide()
    assert is_hidden()
    assert ghoulctl.ping()
