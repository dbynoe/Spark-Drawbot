.PHONY: drawbot kinematics

all: drawbot kinematics

drawbot:
	$(MAKE) -C daemon

kinematics:
	$(MAKE) -C kinematics

clean:
	$(MAKE) -C daemon clean
	$(MAKE) -C kinematics clean
