.PHONY: all module loader clean purge install uninstall

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
	install -m 0644 -T $(MODULEDIR)/ctfmod-stripped.ko \
		/lib/modules/$(shell uname -r)/kernel/drivers/misc/ctfmod.ko
	install -T $(LIBDIR)/loader /usr/local/sbin/ctfmod-loader
	install -m 0644 $(INCLUDEDIR)/ctfmod.h /usr/local/include/
	install -m 0644 $(LIBDIR)/ctfmod-load-secrets.service /etc/systemd/system/
	install -m 0600 -o root -g root -T flags /etc/default/ctfmod-flags
	rmmod ctfmod 2>/dev/null || true
	modprobe ctfmod
	systemctl daemon-reload
	systemctl enable ctfmod-load-secrets.service
	systemctl start ctfmod-load-secrets.service

uninstall:
	systemctl disable ctfmod-load-secrets.service
	rmmod ctfmod
	rm -f /lib/modules/$(shell uname -r)/kernel/drivers/misc/ctfmod.ko \
			/usr/local/sbin/ctfmod-loader \
			/usr/local/include/ctfmod.h \
			/etc/systemd/system/ctfmod-load-secrets.service \
			/etc/default/ctfmod-flags
