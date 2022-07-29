import os
import sys

from utils import output

sys.path.append(os.path.abspath('.'))
import ghoulctl


MODULE_NAME = os.environ.get('GHOUL_MODULE_NAME', 'ghoul')


def test_show_and_hide():
    # make sure conditions are right
    assert ghoulctl.ping()
    assert MODULE_NAME not in output('lsmod')
    
    # perform test
    ghoulctl.show()
    assert MODULE_NAME in output('lsmod')
    assert ghoulctl.ping()

    ghoulctl.hide()
    assert MODULE_NAME not in output('lsmod')
    assert ghoulctl.ping()


def test_show_and_hide_twice():
    # make sure conditions are right
    assert ghoulctl.ping()
    assert MODULE_NAME not in output('lsmod')

    # perform test
    ghoulctl.show()
    ghoulctl.show()
    assert MODULE_NAME in output('lsmod')
    assert ghoulctl.ping()

    ghoulctl.hide()
    ghoulctl.hide()
    assert MODULE_NAME not in output('lsmod')
    assert ghoulctl.ping()
