getflag-obj := ioctl.o sha256.o flagio.o

getflag: $(getflag-obj)
	gcc -o $@ $^

%.o: %.c
	gcc -c -o $@ $<

clean:
	rm -f $(getflag-obj) getflag

.PHONY: clean
