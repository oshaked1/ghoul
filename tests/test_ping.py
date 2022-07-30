import ghoulctl


def test_ping():
    assert ghoulctl.ping()


def test_ping_wrong_arg():
    # make sure conditions are right
    assert ghoulctl.ping()

    # perform test
    assert not ghoulctl.ping(ghoulctl.ping_arg + 1)


def test_change_ping_arg():
    # make sure conditions are right
    assert ghoulctl.ping()

    # perform test
    ghoulctl.change_ping_arg(ghoulctl.ping_arg + 1)

    # ghoulctl keeps track of ping arg so no need to manually set it for next requests
    assert ghoulctl.ping()

    # revert change
    ghoulctl.change_ping_arg(ghoulctl.ping_arg - 1)
    assert ghoulctl.ping()
