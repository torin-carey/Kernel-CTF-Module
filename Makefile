.PHONY: all module clean purge install

moduledir := module
supplieddir := supplied
copyfiles := sha256.h sha256.c ctfmod.h flag2

all: module $(addprefix $(supplieddir)/,$(copyfiles)) ctfmod.ko loader

ctfmod.ko: $(moduledir)/ctfmod.ko
	strip --strip-unneeded -o $@ $<

loader: $(moduledir)/loader
	cp $< $@

module:
	make -C $(moduledir)/ $@

$(supplieddir)/%: $(moduledir)/% | $(supplieddir)
	cp $< $@

$(supplieddir):
	mkdir -p $@

clean purge:
	make -C $(moduledir)/ $@
	rm -f ctfmod.ko flag2gen loader
	rm -rf $(supplieddir)

install: all
	install ctfmod.ko /lib/modules/$(shell uname -r)/kernel/drivers/misc/
	install loader /usr/local/sbin/
	install supplied/ctfmod.h /usr/local/include/
	modprobe ctfmod
