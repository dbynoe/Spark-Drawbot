# This file is part of The Telus Spark Drawbot Code.  It free software: you can
# redistribute it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation, version 2.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
# 
# The project was designed and costructed by David Bynoe - http://www.davidbynoe.com
# This software authored by Kevin Loney - http://brainsinjars.com/
#
# Copyright (C) 2013 David Bynoe

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
