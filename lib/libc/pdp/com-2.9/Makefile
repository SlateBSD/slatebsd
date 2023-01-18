#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.6 (Berkeley) 1/25/87
#
SRCS=	fetchi.s fperr.s gldav.s l3.c loadav.c lock.s phys.s rtp.s ucall.s
OBJS=	fetchi.o fperr.o gldav.o l3.o loadav.o lock.o phys.o rtp.o ucall.o
CFLAGS= -O ${DEFS}
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

com-2.9lib com-2.9lib_p: ${OBJS}
	@echo "building profiled com-2.9"
	@cd profiled; ar cru ../com-2.9lib_p ${OBJS}
	@echo "building normal com-2.9"
	@ar cru com-2.9lib ${OBJS}

tags:

clean:
	rm -f *.o profiled/*.o errs a.out core com-2.9lib com-2.9lib_p \
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

fetchi.o: fetchi.s ./SYS.h /usr/include/syscall.h
fperr.o: fperr.s ./SYS.h /usr/include/syscall.h
gldav.o: gldav.s ./SYS.h /usr/include/syscall.h
l3.o: l3.c
loadav.o: loadav.c
lock.o: lock.s ./SYS.h /usr/include/syscall.h
phys.o: phys.s ./SYS.h /usr/include/syscall.h
rtp.o: rtp.s ./SYS.h /usr/include/syscall.h
ucall.o: ucall.s ./SYS.h /usr/include/syscall.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
