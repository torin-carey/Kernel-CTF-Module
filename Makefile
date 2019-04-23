.PHONY: all module loader docs clean purge install uninstall

CWD := $(shell pwd)

MODULEDIR := $(CWD)/module
INCLUDEDIR := $(CWD)/include
LIBDIR := $(CWD)/lib
DOCDIR := $(CWD)/docs

UDEVDIR := /etc/udev/rules.d
UDEVRULES := $(UDEVDIR)/50-ctfmod.rules

CFLAGS := -O2 -Wall -I"$(INCLUDEDIR)"

export CFLAGS

all: module loader docs flags

flags:
	$(LIBDIR)/genflags.sh >$@

loader:
	make -C $(LIBDIR) loader

module:
	make -C $(MODULEDIR)/ $@

docs: flags
	make -C $(DOCDIR)/ $@

clean:
	make -C $(MODULEDIR)/ $@
	make -C $(LIBDIR)/ $@
	make -C $(DOCDIR)/ $@

purge: clean
	rm -f flags

install: all
	install -m 0644 -T $(MODULEDIR)/ctfmod-stripped.ko \
		/lib/modules/$(shell uname -r)/kernel/drivers/misc/ctfmod.ko
	install -T $(LIBDIR)/loader /usr/local/sbin/ctfmod-loader
	install -m 0644 $(INCLUDEDIR)/ctfmod.h /usr/local/include/
	install -m 0644 $(LIBDIR)/ctfmod-load-secrets.service /etc/systemd/system/
	install -m 0644 -DT $(DOCDIR)/ctfmod.4 /usr/local/man/man4/ctfmod.4
	install -m 0600 -o root -g root -T flags /etc/default/ctfmod-flags
	install -m 0644 $(MODULEDIR)/ctfmod.rules $(UDEVRULES)
	rmmod ctfmod 2>/dev/null || true
	modprobe ctfmod
	systemctl daemon-reload
	systemctl enable ctfmod-load-secrets.service
	systemctl start ctfmod-load-secrets.service

uninstall:
	systemctl disable ctfmod-load-secrets.service || true
	rmmod ctfmod || true
	rm -f /lib/modules/$(shell uname -r)/kernel/drivers/misc/ctfmod.ko \
			/usr/local/sbin/ctfmod-loader \
			/usr/local/include/ctfmod.h \
			/etc/systemd/system/ctfmod-load-secrets.service \
			/usr/local/man/man4/ctfmod.4 \
			/etc/default/ctfmod-flags \
			$(UDEVRULES)
	rmdir --ignore-fail-on-non-empty /usr/local/man/man4/ 2>/dev/null || true
