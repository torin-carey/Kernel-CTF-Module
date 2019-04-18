TARGETS := ctfmod.4

docs: $(TARGETS)

ctfmod.4: ctfmod.md.gen ../flags
	bash $< | pandoc -s -t man >$@

clean purge:
	rm -f $(TARGETS)

.PHONY: docs clean purge
