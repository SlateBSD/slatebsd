#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.2 (Berkeley) 2/6/86
#
ALL=	tftp
DESTDIR=
CFLAGS=-O
SEPFLAG=-i

all: ${ALL}

tftp:	main.o tftp.o tftpsubs.o
	${CC} main.o tftp.o tftpsubs.o ${SEPFLAG} -o tftp

clean:
	rm -f ${ALL} *.o *.s errs core a.out t.?

install: 
	install -s tftp ${DESTDIR}/usr/ucb
