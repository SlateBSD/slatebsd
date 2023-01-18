#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.4 (Berkeley) 2/1/86
#
DESTDIR=
CFLAGS=	-O

# We don't load these separate I&D (-i) or even shared (-n) since
# they weren't to begin with and no one has looked to see if they can
# be loaded that way.  There is a rumor that some of these actually
# modify their instruction sequnce ...
SEPFLAG= -i

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	chess zork

# Assembly programs that live in the current directory and need explicit
# make lines.
#
ASNSTD=	bj factor moo primes ttt

all:	${SUBDIR} ${ASNSTD}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS} SEPFLAG=${SEPFLAG}

FRC:

install:
	-for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done
	-for i in ${ASSTD} ${NSTD}; do \
		(install -s $$i ${DESTDIR}/usr/games/$$i); done

clean:
	rm -f a.out core *.s *.o
	-for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done
	rm -f ${ASNSTD}

depend:

# Files listed in ${ASNSTD} have explicit make lines given below.

bj: bj.s etc.s us.s
	as -o bj.o etc.s bj.s us.s
	ld ${SEPFLAG} -o bj bj.o -lc

factor: factor.s
	as -o factor.o factor.s
	ld ${SEPFLAG} -o factor factor.o -lc

moo: moo.s etc.s
	as -o moo.o etc.s moo.s
	ld ${SEPFLAG} -o moo moo.o -lc

primes: primes.s
	as -o primes.o primes.s
	ld ${SEPFLAG} -o primes primes.o -lc

ttt: ttt.s etc.s
	as -o ttt.o etc.s ttt.s
	ld ${SEPFLAG} -o ttt ttt.o -lc
