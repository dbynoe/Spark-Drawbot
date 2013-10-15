#!/usr/bin/env python
# encoding: utf-8

import logging
import time
import signal
import os
import atexit
import sys
import traceback

import ConfigParser
from emc import EMC
from daemon import runner

def shutdown(app):
	app.shutdown()

class App(object):
	def __init__(self, config):
		self._config = config

		self.stdin_path = '/dev/null'
		self.stdout_path = '/dev/null'
		self.stderr_path = '/dev/null'

		self.pidfile_path =  self._config.get("DAEMON", "pid_file")
		self.pidfile_timeout = 5

	def run(self):
		logger.info("Starting")
		atexit.register(shutdown, self)

		try:
			self.setup()
			while True:
				self.loop()
				time.sleep(10.0)
		except Exception as ex:
			logger.error(ex)

	def setup(self):
		self.emc = EMC(cfg.get("LINUXCNC", "password"))
		self.emc.host = self._config.get("LINUXCNC", "host") 
		self.emc.port = self._config.getint("LINUXCNC", "port")

		self.emc.connect()
		self.emc.hello()

	def loop(self):
		# read and log all errors
		while True:
			error = self.emc.error
			if error == "ERROR OK":
				break
			logger.error(error)

		# check the program status
		program_status = self.emc.program_status
		if program_status == "PROGRAM_STATUS IDLE":
			self.on_idle()
		else:
			pass

	def on_idle(self):
		if self.emc.estop == "ESTOP ON":
			self.emc.estop = False
		
		if self.emc.machine == "MACHINE OFF":
			self.emc.machine = True

		if self.emc.estop and not self.emc.machine:
			logger.warn("Unable to start machine")
			return

		if not all(self.emc.joint_homed()[0:4]):
			logger.info("Waiting for joints to home")
			return

		self.emc.open(self.next_program())
		self.emc.mode = "auto"
		self.emc.run()

	def next_program(self):
		return ""

	def shutdown(self):
		logger.info("Stopping")

cfg = ConfigParser.ConfigParser()
if len(sys.argv) != 3:
	sys.exit(-1)

cfg.read(sys.argv[2])

formatter = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
handler = logging.FileHandler(cfg.get("DAEMON", "log_file"))
handler.setFormatter(formatter)

logger = logging.getLogger("daemon")
logger.setLevel(logging.INFO)
logger.addHandler(handler)

logger = logging.getLogger("emc")
logger.setLevel(logging.INFO)
logger.addHandler(handler)

daemon_runner = runner.DaemonRunner(App(cfg))
daemon_runner.daemon_context.files_preserve=[handler.stream]
daemon_runner.do_action()

