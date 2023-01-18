# If SEPFLAG is not "-i" then DEFS must include "-DNONSEPARATE" on the
# PDP-11.  Ex:
#	SEPFLAG= -n
#	DEFS= -DNONSEPARATE

SEPFLAG= -i
DEFS= 
CFLAGS=	-O ${DEFS}

ALL = dump dumpdir

all: ${ALL}

dump: dump.o
	cc ${SEPFLAG} dump.o -o dump

dumpdir: dumpdir.o
	cc ${SEPFLAG} dumpdir.o -o dumpdir

install:
	install -s dump /etc
	install -s dumpdir /etc

clean:
	rm -f ${ALL} *.o tags

tags:
	rm -f tags
	ctags *.c
