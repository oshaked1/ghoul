import subprocess


def output(command: str):
    return subprocess.check_output([part for part in command.split(' ')])
