obj-m := myflag.o

myflag-objs := flag.o sha256.o

.PHONMY: all

all: modules

modules clean:
	make -C module/ $@
