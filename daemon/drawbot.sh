#! /bin/bash
### BEGIN INIT INFO
# Provides: drawbot
# Required-Start: $all
# Should-Start: 
# Required-Stop: 
# Should-Stop:
# Default-Start:  2 3 4 5
# Default-Stop:   0 1 6
# Short-Description: Drawbot daemon process
# Description: 
### END INIT INFO

#DAEMON=/usr/share/chandelier/chandelier.py
DAEMON=/usr/share/drawbot/daemon.py

case "$1" in
  start)
    echo "Starting server"
    # Start the daemon 
    python $DAEMON start
    ;;
  stop)
    echo "Stopping server"
    # Stop the daemon
    python $DAEMON stop
    ;;
  restart)
    echo "Restarting server"
    python $DAEMON restart
    ;;
  *)
    # Refuse to do other stuff
    echo "Usage: /etc/init.d/drawbot {start|stop|restart}"
    exit 1
    ;;
esac

exit 0