TARGETS := ctfmod.4

docs: $(TARGETS)

ctfmod.4: ctfmod.md
	pandoc $< -s -t man >$@

clean purge:
	rm -f $(TARGETS)
