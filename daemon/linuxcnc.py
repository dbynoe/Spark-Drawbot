import signal
import subprocess
import logging

class LinuxCNC(object):
    def __init__(self, ini):
        self._ini = ini
        self._linuxcnc = None

        self.log = logging.getLogger('linuxcnc')

    def start(self):
        self._linuxcnc = subprocess.Popen(["linuxcnc", self._ini], )

    def halfile(self, path):
        ret = subprocess.call(['halcmd', '-f', path])

    def halcmd(self, cmd):
        cmd = cmd.split(' ')
        cmd.insert(0, 'halcmd')
        ret = subprocess.call(cmd)

    @property
    def running(self):
        if not self._linuxcnc:
            return False
        return not self._linuxcnc.poll()
