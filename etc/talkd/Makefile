#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.4 (Berkeley) 3/13/86
#
DESTDIR=
OBJS=	talkd.o
CFLAGS= -O
SEPFLAG= -i

talkd: ${OBJS}
	${CC} ${SEPFLAG} ${CFLAGS} -o talkd ${OBJS}

install: talkd
	install -s talkd ${DESTDIR}/etc/ntalkd

clean:
	rm -f ${OBJS} errs core a.out talkd

tags:	${SRCS}
	ctags ${SOURCE}
