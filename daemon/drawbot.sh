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

#! /bin/bash
### BEGIN INIT INFO
# Provides: drawbot
# Required-Start: $all
# Required-Stop: 
# Default-Start: 2 3 4 5
# Default-Stop: 0 1 6
# Short-Description: Drawbot daemon process
### END INIT INFO

#DAEMON=/usr/share/chandelier/chandelier.py
DAEMON=/usr/share/drawbot/daemon/drawbot.py
CONFIG=/usr/share/drawbot/daemon/drawbot.config

case "$1" in
  start)
    echo "Starting server"
    # Start the daemon 
    python $DAEMON start $CONFIG
    ;;
  stop)
    echo "Stopping server"
    # Stop the daemon
    python $DAEMON stop $CONFIG
    sleep 10s
    ;;
  restart)
    echo "Restarting server"
    python $DAEMON restart $CONFIG
    ;;
  *)
    # Refuse to do other stuff
    echo "Usage: /etc/init.d/drawbot {start|stop|restart}"
    exit 1
    ;;
esac

exit 0
