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
# Copyright (C) 2013 David Bynoe and Kevin Loney
import time
from linuxcnc import LinuxCNC

cnc = LinuxCNC("/usr/share/drawbot/kinematics/Drawbot4Axis/Drawbot4Axis.ini")
cnc.start()
time.sleep(20)
cnc.stop()
