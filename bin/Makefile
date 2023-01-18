#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.19 (Berkeley) 5/22/86
#
DESTDIR=
CFLAGS=	-O
SEPFLAG= -i

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	adb as awk csh diff make sed sh tp

# Shell scripts that need only be installed and are never removed.
#
SCRIPT=	false true

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	ar cat cc chgrp chmod cmp cp date dd du echo ed grep hostid \
	hostname kill ld ln ls mkdir mt mv nice nm od pagesize pr \
	pwd rm rmail rmdir size strip stty sync tar tee test time who

# C programs that live in the current directory and need explicit make lines.
#
NSTD=	expr

# Programs that must run setuid to root
#
SETUID=	login mail passwd rcp su

# Programs that run set-group-id operator
#
OPERATOR= df

# Programs that must run set-group-id kmem.
#
KMEM=	ps

# Programs that must run set-group-id tty.
#
TTY=	wall write

BINS=	${STD} ${NSTD} ${SETUID} ${OPERATOR} ${KMEM} ${TTY}

all:	${SUBDIR} ${BINS}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS} SEPFLAG=${SEPFLAG}

FRC:


${STD} ${SETUID} ${OPERATOR} ${TTY}:
	${CC} ${CFLAGS} ${SEPFLAG} -o $@ $@.c

install:
	install -s ${STD} ${NSTD} ${DESTDIR}/bin
	-for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done
	-for i in ${SCRIPT}; do (install -c $$i.sh ${DESTDIR}/bin/$$i); done
	-for i in ${SETUID}; do \
		(install -o root -m 4755 -s $$i ${DESTDIR}/bin/$$i); done
	-for i in ${OPERATOR}; do \
		(install -g operator -m 2755 -s $$i ${DESTDIR}/bin/$$i); done
	-for i in ${KMEM}; do \
		(install -g kmem -m 2755 -s $$i ${DESTDIR}/bin/$$i); done
	-for i in ${TTY}; do \
		(install -g tty -m 2755 -s $$i ${DESTDIR}/bin/$$i); done
	rm -f ${DESTDIR}/bin/[; ln ${DESTDIR}/bin/test ${DESTDIR}/bin/[
	rm -f ${DESTDIR}/bin/e; ln ${DESTDIR}/bin/ed ${DESTDIR}/bin/e
	rm -f ${DESTDIR}/bin/chfn; ln ${DESTDIR}/bin/passwd ${DESTDIR}/bin/chfn
	rm -f ${DESTDIR}/bin/chsh; ln ${DESTDIR}/bin/passwd ${DESTDIR}/bin/chsh

clean:
	rm -f ${BINS} expr.c a.out core *.s *.o errs
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

depend: expr.c
	for i in ${BINS}; do \
	    cc -M ${INCPATH} $$i.c | sed 's/\.o//' | \
	    awk ' { if ($$1 != prev) \
		{ if (rec != "") print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		else rec = rec " " $$2 } } \
		END { print rec } ' >> makedep; done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# Files listed in ${NSTD} have explicit make lines given below.

expr: expr.o
	${CC} ${CFLAGS} ${SEPFLAG} -o expr expr.o

ps:
	${CC} ${CFLAGS} ${SEPFLAG} -o ps ps.c -lm

# DO NOT DELETE THIS LINE -- make depend uses it

