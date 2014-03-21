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


import logging
import sys
import uuid
import telnetlib

def map_constant(val):
	if val == "YES" or val == "ON":
		return True
	return False

class EMC(object):
	HELLO = "hello {password} {client} 1.0\r\n"
	GET = "get {sub} {params}\r\n"
	SET = "set {sub} {params}\r\n"
	QUIT = "quit\r\n"
	SHUTDOWN = "shutdown\r\n"

	def __init__(self, password):
		self._auth_client = uuid.uuid4()
		self._auth_password = password

		self._host = "localhost"
		self._port = 5007

		self._logger = logging.getLogger("emc")

	@property
	def host(self):
		return self._host
	@host.setter
	def host(self, value):
		self._host = value

	@property
	def port(self):
		return self._port
	@port.setter
	def port(self, value):
		self._port = value

	def connect(self):
		self._telnet = telnetlib.Telnet(self._host, self._port)
		self._telnet.set_option_negotiation_callback(self.negotiation_callback)

	def negotiation_callback(self, socket, command, option):
		self._logger.info(command)
		self._logger.info(option)

	def execute(self, command):
		self._logger.debug(command.strip())
		self._telnet.write(command)

		try:
			result = self._telnet.read_until("\r\n").strip()
		except EOFError as ex:
			self._logger.error(ex)
			return None

		return result

	def hello(self):
		result = self.execute(self.HELLO.format(password = self._auth_password, client = self._auth_client))
		if not result.startswith("HELLO ACK"):
			sys.exit(-1)
		self.set("enable", "EMCTOO")

	def get(self, sub, *params):
		self.execute(self.GET.format(sub = sub, params = " ".join(map(str, params))))

		result = self._telnet.read_until("\r\n").strip()
		self._logger.debug(result)

		return result

	def set(self, sub, *params):
		return self.execute(self.SET.format(sub = sub, params = " ".join(map(str, params))))

	def quit(self):
		return self.execute(self.QUIT)

	def shutdown(self):
		return self.execute(self.SHUTDOWN)

	def open(self, file):
		return self.set("open", file)

	def run(self):
		return self.set("run")

	def abort(self):
		return self.set("abort")

	def mdi(self, cmd):
		return self.set("mdi", cmd)

	def joint_homed(self, joint = None):
		if joint is None:
			joints = self.get("joint_homed").split(" ")
			joints.remove("JOINT_HOMED")
			return map(map_constant, joints)
		else:
			return self.get("joint_homed", joint)

	@property
	def program_status(self):
		return self.get("program_status")

	@property
	def error(self):
		return self.get("error")

	# estop
	@property
	def estop(self):
		return self.get("estop")
	@estop.setter
	def estop(self, value):
		self.set("estop", "on" if value else "off")

	# machine
	@property
	def machine(self):
		return self.get("machine")
	@machine.setter
	def machine(self, value):
		self.set("machine", "on" if value else "off")

	# mode
	@property
	def mode(self):
		return self.get("mode")
	@mode.setter
	def mode(self, value):
		self.set("mode", value)

	#teleop
	@property
	def teleop(self):
		return self.get("teleop_enable")
	@mode.setter
	def teleop(self, value):
		self.set("teleop_enable", "on" if value else "off")
