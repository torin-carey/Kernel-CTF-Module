.PHONY: all module clean purge

moduledir := module
supplieddir := supplied
copyfiles := sha256.h sha256.c ioctl.h flag2

all: module $(addprefix $(supplieddir)/,$(copyfiles))

module:
	make -C $(moduledir)/ $@
	ln -fs $(moduledir)/ctfmod.ko .

$(supplieddir)/%: $(moduledir)/% | $(supplieddir)
	cp $< $@

$(supplieddir):
	mkdir -p $@

clean purge:
	make -C $(moduledir)/ $@
	rm -f ctfmod.ko flag2gen
	rm -rf $(supplieddir)
