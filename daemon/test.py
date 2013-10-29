import time
from linuxcnc import LinuxCNC

cnc = LinuxCNC("/usr/share/drawbot/kinematics/Drawbot4Axis/Drawbot4Axis.ini")
cnc.start()
time.sleep(20)
cnc.stop()
