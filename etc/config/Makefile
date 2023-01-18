#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.3 (Berkeley) 2/18/86
#
#
CFILES=	config.c
OBJS=	config.o
SEPFLAG= -i

CFLAGS=-O
LDFLAGS=-ll

config: ${OBJS}
	${CC} ${OBJS} ${SEPFLAG} -o config ${LDFLAGS}

clean:
	rm -f ${OBJS} lex.yy.c y.tab.c y.tab.h config

install:
	install -s config ${DESTDIR}/etc/config

lint: ${CFILES}
	lint ${CFILES}

depend:
	grep '^#include' ${CFILES} | grep -v '<' | \
	sed -e 's/:[^"]*"\([^"]*\)".*/: \1/' \
	    -e 's/\.c/.o/' \
	    -e 's,../[a-zA-Z]*/,,' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '$$r makedep' >>eddep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep

# DO NOT DELETE THIS LINE -- make depend uses it

