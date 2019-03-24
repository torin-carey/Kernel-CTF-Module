.PHONY: all module clean purge

moduledir := module
supplieddir := supplied
copyfiles := sha256.h sha256.c ioctl.h flag2

all: module $(addprefix $(supplieddir)/,$(copyfiles)) ctfmod.ko

ctfmod.ko: $(moduledir)/ctfmod.ko
	ln -f $< $@

module:
	make -C $(moduledir)/ $@

$(supplieddir)/%: $(moduledir)/% | $(supplieddir)
	cp $< $@

$(supplieddir):
	mkdir -p $@

clean purge:
	make -C $(moduledir)/ $@
	rm -f ctfmod.ko flag2gen
	rm -rf $(supplieddir)
