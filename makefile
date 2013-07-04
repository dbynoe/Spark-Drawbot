SUBDIRS=daemon kinematics
TARGET=

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(TARGET)

install: TARGET=install
install: $(SUBDIRS)

clean: TARGET=clean
clean: $(SUBDIRS)
