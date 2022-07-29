import os
import sys
from time import sleep

sys.path.append(os.path.abspath('.'))
import ghoulctl


MODULE_NAME = os.environ.get('GHOUL_MODULE_NAME', 'ghoul')


def test_unload():
    # make sure conditions are right
    assert ghoulctl.ping()

    # perform test
    ghoulctl.unload()
    sleep(0.5)
    assert not ghoulctl.ping()

    # reload ghoul
    os.system(f'sudo insmod {MODULE_NAME}.ko')
    assert ghoulctl.ping()
