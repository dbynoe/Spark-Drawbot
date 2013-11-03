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