import uuid
import telnetlib

class EMC:
	HELLO = "hello {auth[password]} {auth[client]} 1.0\r\n"
	GET = "get {sub} {params}\r\n"
	SET = "set {sub} {params}\r\n"
	QUIT = "quit\r\n"
	SHUTDOWN = "shutdown\r\n"

	def __init__(self, host, port, password):
		self.auth = {}
		self.auth["client"] = uuid.uuid4()
		self.auth["password"] = password

		self.telnet = telnetlib.Telnet(host, port)

	def execute(self, command):
		self.telnet.write(command)
		return self.telnet.read_until("\r\n")

	def hello(self):
		self.execute(self.HELLO.format(auth = self.auth))
		self.set("enable", "EMCTOO")

	def get(self, sub, *params):
		return self.execute(self.GET.format(sub = sub, params = " ".join(map(str, params))))

	def set(self, sub, *params):
		return self.execute(self.SET.format(sub = sub, params = " ".join(map(str, params))))

	def quit(self):
		self.execute(QUIT)

	def shutdown(self):
		self.execute(SHUTDOWN)
