#
# Makefile for the system autoconfiguration program
#
DESTDIR=
N=	../vaxif
M=	../machine
D=	../pdpuba

OBJS=	attach.o do_config.o main.o read_dtab.o read_nlist.o uprobe.o	\
	cnauto.o dhauto.o dhuauto.o drauto.o dzauto.o hkauto.o htauto.o	\
	lpauto.o raauto.o rkauto.o rlauto.o siauto.o tmauto.o tsauto.o	\
	xpauto.o rxauto.o
SRCS=	attach.c do_config.c main.c read_dtab.c read_nlist.c uprobe.c	\
	${M}/cnauto.c ${D}/dhauto.c ${D}/dhuauto.c ${D}/drauto.c	\
	${D}/dzauto.c ${D}/hkauto.c ${D}/htauto.c ${D}/lpauto.c		\
	${D}/raauto.c ${D}/rkauto.c ${D}/rlauto.c ${D}/siauto.c		\
	${D}/tmauto.c ${D}/tsauto.c ${D}/xpauto.c ${D}/rxauto.c

VPATH=	.:../vaxif:../machine:../pdpuba
CFLAGS=	-O -I. -I../h
LFLAG=	-i

autoconfig: ${OBJS}
	${CC} ${OBJS} -o $@

install:
	install -s -m 700 autoconfig ${DESTDIR}/etc

depend: 
	${CC} -M ${CFLAGS} ${SRCS} | \
	sed -e ':loop' \
	    -e 's/\.\.\/[^ /]*\/\.\./../' \
	    -e 't loop' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	ex - Makefile < eddep
	rm eddep makedep

tags: FRC
	rm -f tags
	/usr/ucb/ctags ${SRCS}

clean: FRC
	rm -f autoconfig *.o

FRC:

# DO NOT DELETE THIS LINE

