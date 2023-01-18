#	Makefile	4.1	83/08/11
#
CFLAGS = -O
SEPFLAG= -i

all: lex

lex: lmain.o y.tab.o sub1.o sub2.o header.o
	-if [ X${SEPFLAG} = X-i ]; then \
		cc ${SEPFLAG} lmain.o y.tab.o sub1.o sub2.o header.o -o lex; \
	else \
		ld ${SEPFLAG} -X /lib/crt0.o \
		-Z lmain.o y.tab.o \
		-Z sub2.o \
		-Z header.o \
		-Y sub1.o -lc -o lex; \
	fi

smallex:
	-if [ X${SEPFLAG} = X-i ]; then \
		cc ${SEPFLAG} lmain.o y.tab.o sub1.o sub2.o header.o -o lex; \
	else \
		ld ${SEPFLAG} -X /lib/crt0.o \
		-Z lmain.o y.tab.o \
		-Z sub2.o \
		-Z header.o \
		-Y sub1.o -lc -o lex; \
	fi

y.tab.c: parser.y
	yacc parser.y
lmain.o:lmain.c ldefs.c once.c
	cc -c -O lmain.c
sub1.o: sub1.c ldefs.c
	cc -c -O sub1.c
sub2.o: sub2.c ldefs.c
	cc -c -O sub2.c
header.o: header.c ldefs.c
	cc -c -O header.c

install: all
	install -s lex $(DESTDIR)/usr/bin
	install -c ncform $(DESTDIR)/usr/lib/lex
	install -c nrform $(DESTDIR)/usr/lib/lex

clean:
	rm -f *.o lex y.tab.c
