.PHONY: all module loader clean purge install

CWD := $(shell pwd)

MODULEDIR := $(CWD)/module
INCLUDEDIR := $(CWD)/include
LIBDIR := $(CWD)/lib

CFLAGS := -I"$(INCLUDEDIR)"

export CFLAGS
all: module loader flags

flags:
	$(LIBDIR)/genflags.sh >$@

loader:
	make -C $(LIBDIR) loader

module:
	make -C $(MODULEDIR)/ $@

clean:
	make -C $(MODULEDIR)/ $@
	make -C $(LIBDIR)/ $@

purge: clean
	rm -f flags

install: all
	install -T $(MODULEDIR)/ctfmod-stripped.ko \
		/lib/modules/$(shell uname -r)/kernel/drivers/misc/ctfmod.ko
	install -T $(LIBDIR)/loader /usr/local/sbin/ctfmod-loader
	install $(INCLUDEDIR)/ctfmod.h /usr/local/include/
	install -m 0600 -o root -g root -T flags /etc/default/ctfmod-flags
	modprobe ctfmod
