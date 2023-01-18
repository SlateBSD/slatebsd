# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.3 (Berkeley) 9/7/85
#
ALL=	implog implogd
DESTDIR=
CFLAGS=-O
SEPFLAG= -i

all:	${ALL}

implog:	implog.o
	${CC} ${SEPFLAG} implog.o -o implog

implogd: implogd.o
	${CC} ${SEPFLAG} implogd.o -o implogd

clean:
	rm -f ${ALL} *.o *.s errs core a.out t.?

install: ${ALL}
	install -s implog ${DESTDIR}/etc
	install -s implogd ${DESTDIR}/etc
