#
# crash makefile
#

DESTDIR=

CFLAGS=	-O
SEPFLAG=-i

HDRS=	crash.h
#SRCS=	crash.c crashsubs.c dispnet.c symbol.c
#OBJS=	crash.o crashsubs.o dispnet.o symbol.o
SRCS=	crash.c crashsubs.c dispnet.c symbol.c if.c inet.c route.c
OBJS=	crash.o crashsubs.o dispnet.o symbol.o if.o inet.o route.o

crash: ${OBJS}
	${CC} -o $@ ${SEPFLAG} ${OBJS}

install: crash crash.1
	install -s crash ${DESTDIR}/usr/new/crash
	install -c crash.1 ${DESTDIR}/usr/man/mann/crash.n

tags: ${HDRS} ${SRCS}
	ctags ${HDRS} ${SRCS}

clean:
	rm -f *.o crash tags core

crash.o:	crash.c crash.h
crashsubs.o:	crashsubs.c crash.h
dispnet.o:	dispnet.c crash.h
symbol.o:	symbol.c crash.h
