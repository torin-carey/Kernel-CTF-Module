.PHONY: all module clean purge

moduledir := module
supplieddir := supplied
copyfiles := sha256.h sha256.c ioctl.h flag2

all: module $(addprefix $(supplieddir)/,$(copyfiles)) ctfmod.ko loader

ctfmod.ko: $(moduledir)/ctfmod.ko
	ln -f $< $@

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
