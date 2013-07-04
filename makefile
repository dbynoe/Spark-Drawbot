SUBDIRS=daemon kinematics
TARGET=

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(TARGET)


clean: TARGET=clean
clean: $(SUBDIRS)
