import os
from time import sleep
import subprocess
import pytest

import ghoulctl


WARNING = '\033[93m'
ENDC = '\033[0m'
MODULE_NAME = os.environ.get('GHOUL_MODULE_NAME', 'ghoul')


@pytest.mark.last
def test_ftrace_works():
    # make sure ghoul is loaded
    assert ghoulctl.ping()

    # print warning message, because if this test fails the machine will freeze when ghoul is unloaded
    print(f"{WARNING}Warning: ftrace is being enabled. If this test fails, the machine will freeze and will need to be reset!!!{ENDC}")

    # enable ftrace
    os.system('sudo bash -c "echo function > /sys/kernel/debug/tracing/current_tracer"')

    # make sure ftrace did not error
    output = subprocess.check_output(['bash', '-c', 'dmesg | tail -10']).decode()
    error = 'ftrace faulted on writing' in output
    
    # try unloading ghoul
    ghoulctl.unload()
    sleep(0.1) # may need to increase time if next assertion fails
    assert not ghoulctl.ping()

    assert not error
    
    # disable ftrace
    os.system('sudo bash -c "echo nop > /sys/kernel/debug/tracing/current_tracer"')

    # reload ghoul
    os.system(f'sudo insmod {MODULE_NAME}.ko')
    assert ghoulctl.ping()
