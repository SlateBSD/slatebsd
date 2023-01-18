#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.6 (Berkeley) 6/6/87
#
FPSRCS=	fp0.s fp1_nonsep.s fp1_sep.s fp2.s fp3.s
SRCS=	ffpsim.s fpsim.s setfpt.s
OBJS=	ffpsim.o fpsim.o setfpt.o
CFLAGS=	-O ${DEFS}
TAGSFILE=tags

.s.o:
	/lib/cpp -E ${DEFS} $*.s | sed -e 's;^#;/;' > x.s
	as -V -o $*.o x.s
	-ld -x -r $*.o
	mv a.out $*.o
	rm -f x.s

fpsimlib fpsimlib_p: ${OBJS}
	ar cru fpsimlib ${OBJS}
	cp fpsimlib fpsimlib_p

tags:

clean:
	rm -f *.o profiled/*.o errs a.out core fpsimlib fpsimlib_p \
		tags Makefile.bak

depend:
