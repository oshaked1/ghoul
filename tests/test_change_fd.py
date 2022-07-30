import ghoulctl


def test_change_fd():
    # make sure conditions are right
    assert ghoulctl.ping()

    # perform test
    ghoulctl.change_fd(ghoulctl.service_fd + 1)
    
    # ghoulctl keeps track of fd so no need to manually set it for next requests
    assert ghoulctl.ping()

    # revert change
    ghoulctl.change_fd(ghoulctl.service_fd - 1)
    assert ghoulctl.ping()
