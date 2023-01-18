#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.6 (Berkeley) 1/25/87
#
CFLAGS=	-O ${DEFS}
STDSRCS=rdwrv.c
STD=	rdwrv.o
# these should don't really work like their 4.3 counterparts, but in the
# interest of compatibility...
SRCS=	${STDSRCS}
OBJS=	${STD}
TAGSFILE=tags

.s.o:
	/lib/cpp -E -DPROF ${DEFS} $*.s | sed -e 's;^#;/;' > x.s
	as -V -o $*.o x.s
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	/lib/cpp -E ${DEFS} $*.s | sed -e 's;^#;/;' > x.s
	as -V -o $*.o x.s
	-ld -x -r $*.o
	mv a.out $*.o
	rm -f x.s

.c.o:
	cc -p -c ${CFLAGS} $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	cc -c ${CFLAGS} $*.c
	-ld -x -r $*.o
	mv a.out $*.o

com-4.3lib com-4.3lib_p: ${OBJS}
	@echo "building profiled com-4.3lib"
	@cd profiled; ar cru ../com-4.3lib_p ${OBJS}
	@echo "buiding normal com-4.3lib"
	@ar cru com-4.3lib ${OBJS}

tags:
	cwd=`pwd`; \
	for i in ${SRCS}; do \
		ctags -a -f ${TAGSFILE} $$cwd/$$i; \
	done

clean:
	rm -f *.o profiled/*.o errs a.out core com-4.3lib com-4.3lib_p \
		tags Makefile.bak

depend:
	for i in ${SRCS}; do \
	    cc -M ${CFLAGS} $$i | awk ' { if ($$1 != prev) \
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

# DO NOT DELETE THIS LINE -- make depend uses it

rdwrv.o: rdwrv.c /usr/include/sys/param.h /usr/include/sys/types.h
rdwrv.o: /usr/include/signal.h /usr/include/machine/machparam.h
rdwrv.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
rdwrv.o: /usr/include/sys/ttydev.h /usr/include/sys/uio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
