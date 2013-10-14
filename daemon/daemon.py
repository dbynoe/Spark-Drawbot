#!/usr/bin/env python
# encoding: utf-8

import logging
import time
import signal
import os
import atexit
import sys
import traceback
import telnetlib

import ConfigParser
from emc import EMC

cfg = ConfigParser.ConfigParser()
if len(sys.argv) != 2:
	system.exit(-1)

cfg.read(sys.argv[1])

def shutdown(app):
	app.shutdown()

class App():
	def __init__(self):
		self.stdin_path = '/dev/null'
		self.stdout_path = '/dev/null'
		self.stderr_path = '/dev/null'
		self.pidfile_path =  cfg.get("DAEMON", "pid_file")
		self.pidfile_timeout = 5

	def run(self):
		logger.info("Starting")
		atexit.register(shutdown, self)

		self.setup()
		while True:
			self.loop()
			time.sleep(1.0)

	def setup(self):
		self.emc = EMC(cfg.get("LINUXCNC", "host"), cfg.getint("LINUXCNC", "port"), cfg.get("LINUXCNC", "password"))
		#self.emc.set("mode", "manual")
		self.emc.set("estop", "off")
		self.emc.set("machine", "on")

	def loop(self):
		# read and log all errors
		error = self.emc.get("error")

		# check the program status
		program_status = self.emc.get("program_status")
		if program_status == "idle":
			self.emc.set("open", "")
			self.emc.set("run")

	def shutdown(self):
		logger.info("Stopping")

logger = logging.getLogger("daemon")
logger.setLevel(logging.INFO)

formatter = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
handler = logging.FileHandler(cfg.get("DAEMON", "log_file"))
handler.setFormatter(formatter)
logger.addHandler(handler)

daemon_runner = runner.DaemonRunner(App())
daemon_runner.daemon_context.files_preserve=[handler.stream]
daemon_runner.do_action()
