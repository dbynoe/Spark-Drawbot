TARGET=
SUBDIRS=daemon kinematics

.PHONY: $(SUBDIRS) run

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(TARGET)

install: TARGET=install
install: $(SUBDIRS)

clean: TARGET=clean
clean: $(SUBDIRS)

# Must be run with -j2 flag to function correctly
run: linuxcnc halcmd

halcmd:
	@sleep 10
	halcmd -f /usr/share/drawbot/kinematics/Drawbot4Axis/custom_postgui.hal

linuxcnc:
	linuxcnc /usr/share/drawbot/kinematics/Drawbot4Axis/Drawbot4Axis.ini