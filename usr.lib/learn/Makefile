# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	4.3	(Berkeley)	7/24/87
#
LESSONS=C editor eqn files macros morefiles vi
FILES=	Linfo Xinfo
DESTDIR=

all:

install: ${LESSONS} ${FILES}
	-mkdir ${DESTDIR}/usr/lib/learn \
		${DESTDIR}/usr/lib/learn/log \
		${DESTDIR}/usr/lib/learn/bin
	chown root.staff ${DESTDIR}/usr/lib/learn \
		${DESTDIR}/usr/lib/learn/log \
		${DESTDIR}/usr/lib/learn/bin
	chmod 755 ${DESTDIR}/usr/lib/learn \
		${DESTDIR}/usr/lib/learn/log \
		${DESTDIR}/usr/lib/learn/bin
	@for i in ${LESSONS}; do \
		touch ${DESTDIR}/usr/lib/learn/log/$$i; \
		chmod 666 ${DESTDIR}/usr/lib/learn/log/$$i; \
	done
	@for i in ${FILES}; do \
		echo copy $$i; \
		install -c -m 644 $$i ${DESTDIR}/usr/lib/learn/$$i; \
	done
# For some unknown reason the following line fails with an exit -115 ...
#	tar cf - ${LESSONS} | (cd ${DESTDIR}/usr/lib/learn; tar xpf -)
	-chmod 555 ${DESTDIR}/usr/lib/learn/*/Init
	cd ${DESTDIR}/usr/lib/learn/C; cc -O -c getline.c getnum.c

clean:
