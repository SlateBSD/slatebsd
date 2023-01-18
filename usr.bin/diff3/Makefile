#	Makefile	4.1	83/02/10

CFLAGS=-O
SEPFLAG= -i

diff3: diff3.o
	cc ${SEPFLAG} -o diff3 diff3.o

install: diff3
	install -s diff3 $(DESTDIR)/usr/lib
	install -c diff3.sh $(DESTDIR)/usr/bin/diff3

clean:
	rm -f diff3.o diff3	
