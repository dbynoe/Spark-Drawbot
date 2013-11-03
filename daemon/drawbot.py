#!/usr/bin/env python
# encoding: utf-8

import logging
import time
import signal
import os
import atexit
import sys
import traceback
import socket

import ConfigParser
from emc import EMC
from linuxcnc import LinuxCNC
from random import choice
from daemon import runner

def shutdown(app):
	app.shutdown()

class App(object):
	def __init__(self, config):
		self._config = config
		self._is_homing = False

		self._cnccfg = ConfigParser.ConfigParser()
		self._cnccfg.read(self._config.get("LINUXCNC", "ini"))

		self._hx = 0.0 #0.5 * self._cnccfg.getfloat("DRAWBOT", "LIMIT_X")
		self._hy = 0.0 #0.5 * self._cnccfg.getfloat("DRAWBOT", "LIMIT_Y")
		self._hz = 0.25 * self._cnccfg.getfloat("DRAWBOT", "DIMENSION_Z")

		self.log = logging.getLogger('daemon')

		self.stdin_path = '/dev/null'
		self.stdout_path = '/dev/null'
		self.stderr_path = '/dev/null'

		self.pidfile_path =  self._config.get("DAEMON", "pid_file")
		self.pidfile_timeout = 5

	def run(self):
		logger.info("Starting")
		atexit.register(shutdown, self)

		time.sleep(20.0)

		self.setup();
		try:
			while True:
				while not os.path.exists(self._config.get("LINUXCNC", "lock_file")):
					self.log.info("Starting linuxcnc")
					self.cnc.start()
					time.sleep(5.0)

				self.cnc.halfile(self._config.get("LINUXCNC", "hal_file"))

				while True:
					try:
						self.emc.connect()
						self.emc.hello()
						break
					except socket.error as ex:
						logger.error(ex)
						time.sleep(10.0)

				self._is_drawing = False
				while True:
					try:
						self.loop()
					except EOFError as ex:
						logger.error(ex)
						break
					time.sleep(1.0)
		except Exception as ex:
			logger.error(ex)

	def setup(self):
		self.emc = EMC(cfg.get("LINUXCNC", "password"))
		self.emc.host = self._config.get("LINUXCNC", "host") 
		self.emc.port = self._config.getint("LINUXCNC", "port")

		self.cnc = LinuxCNC(cfg.get("LINUXCNC", "ini"))

	def loop(self):
		# read and log all errors
		while True:
			error = self.emc.error
			if error == "ERROR OK":
				break
			self.log.error(error)

		# check the program status
		program_status = self.emc.program_status
		if program_status == "PROGRAM_STATUS IDLE":
			self.on_idle()
		else:
			pass

	def on_idle(self):
		if self.emc.estop == "ESTOP ON":
			self.log.info("Reseting estop")
			self.emc.estop = False

		if self.emc.machine == "MACHINE OFF":
			self.log.info("Powering on machine")
			self.emc.machine = True

		homed = self.emc.joint_homed()[0:4]
		if all(homed):
			self.cnc.halcmd('setp drawbot.is-homing 0')
			self.cnc.halcmd('setp drawbot.mode.joint 0')
			self.cnc.halcmd('setp drawbot.mode.teleop 1')
		else:
			if not self._is_homing:
				self.log.info("Waiting for joints to home")

				self.cnc.halcmd('setp halui.mode.teleop 0')
				self.cnc.halcmd('setp halui.mode.joint 1')
				for joint, is_homed in enumerate(homed):
					if not is_homed:
						self.cnc.halcmd('setp halui.joint.%d.unhome 1' % joint)
						self.cnc.halcmd('setp halui.joint.%d.unhome 0' % joint)

				self.cnc.halcmd('setp drawbot.is-homing 1')
			self._is_homing = True
			return
		if self._is_homing:
			time.sleep(10)
		self._is_homing = False

		self.gotozero()
		time.sleep(10)

		# Load and run the next program
		next = self.next_program()
		if next:
			self.log.info("Open " + next)
			#self.emc.teleop = True
			self.emc.mode = "auto"
			self.emc.open(next)
			self.emc.run()
		else:
			self.log.warn("No drawing files found")

	def next_program(self):
		if not self._is_drawing:
			source = self._config.get("PATHS", "erase")
			source = self.random_file(source)
			if not source:
				self._is_drawing = True

		if self._is_drawing:
			source = self._config.get("PATHS", "draw")
			source = self.random_file(source)

		if source:
			self._is_drawing = not self._is_drawing

		return source

	def random_file(self, path):
		allFiles = []
		for root, dirs, files in os.walk(path):
			if not root.endswith('/'):
				root += '/'

			#skip all directors that start with '.'
			map(dirs.remove, [d for d in dirs if d.startswith('.')])
			#skip all files that start with '.'
			map(files.remove, [f for f in files if f.startswith('.') and not f.endswith(".ngc")])

			allFiles += [root + f for f in files]

		if allFiles:
			return choice(allFiles)

		return None

	def gotozero(self):
		self.emc.mode = "mdi"
		self.emc.mdi("G0 X{x} Y{y} Z{z}".format(x=self._hx, y=self._hy, z=self._hz))

	def shutdown(self):
		self.log.info("Stopping")
		try:
			self.emc.abort()
			self.gotozero()

			time.sleep(5)
		except Exception as ex:
			logger.error(ex)

cfg = ConfigParser.ConfigParser()
if len(sys.argv) != 3:
	sys.exit(-1)

cfg.read(sys.argv[2])

formatter = logging.Formatter("%(asctime)s %(name)s %(levelname)s - %(message)s")
handler = logging.FileHandler(cfg.get("DAEMON", "log_file"))
handler.setFormatter(formatter)

logger = logging.getLogger()
logger.setLevel(logging.INFO)
logger.addHandler(handler)

daemon_runner = runner.DaemonRunner(App(cfg))
daemon_runner.daemon_context.files_preserve=[handler.stream]
daemon_runner.do_action()
