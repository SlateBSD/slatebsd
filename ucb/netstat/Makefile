#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.5 (Berkeley) 1/18/86
#
OBJS=	host.o inet.o if.o main.o mbuf.o route.o unix.o ns.o
CFLAGS=-O
LFLAG=	-i
DESTDIR=

netstat: ${OBJS}
	${CC} ${LFLAG} ${OBJS} -o netstat

install: netstat
	install -s -m 2755 -g kmem netstat ${DESTDIR}/usr/ucb/netstat

clean:
	rm -f *.o core a.out errs netstat
