#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	4.6	(Berkeley)	6/18/87
#
# Flag BSD4_2 means 4.2 or later.
CFLAGS=	-O -DBSD4_2
SEPFLAG=-i
LIBC=	/lib/libc.a
LLIB=	${DESTDIR}/usr/lib/learn/bin
SRCS=	copy.c dounit.c getlesson.c learn.c list.c mem.c makpipe.c maktee.c \
	mysys.c selsub.c selunit.c start.c whatnow.c wrapup.c
OBJS=	copy.o dounit.o getlesson.o learn.o list.o mem.o makpipe.o maktee.o \
	mysys.o selsub.o selunit.o start.o whatnow.o wrapup.o

all: learn lrntee lcount

learn: ${OBJS} ${LIBC}
	${CC} ${SEPFLAG} -o $@ ${CFLAGS} ${OBJS}

lcount: lcount.c ${LIBC}
	${CC} ${CFLAGS} ${SEPFLAG} -o $@ lcount.c

lrntee: lrntee.c ${LIBC}
	${CC} ${CFLAGS} ${SEPFLAG} -o $@ lrntee.c

clean: FRC
	rm -f ${OBJS} core learn lrntee lcount

depend: FRC
	mkdep ${CFLAGS} ${SRCS} lcount.c lrntee.c

install: FRC
	install -c -s -m 755 learn ${DESTDIR}/usr/bin/learn
	install -s -m 755 learn ${LLIB}/learn
	install -s -m 755 lrntee ${LLIB}/lrntee
	install -s -m 755 lcount ${LLIB}/lcount

lint: FRC
	lint ${CFLAGS} ${SRCS}
	lint ${CFLAGS} lcount.c
	lint ${CFLAGS} lrntee.c

tags: FRC
	ctags ${SRCS}
	ctags -a lcount.c
	ctags -a lrntee.c
	sort -o tags tags

FRC:

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

copy.o: copy.c /usr/include/stdio.h /usr/include/signal.h
copy.o: /usr/include/machine/trap.h lrnref.h
dounit.o: dounit.c /usr/include/stdio.h lrnref.h
getlesson.o: getlesson.c /usr/include/stdio.h lrnref.h
learn.o: learn.c /usr/include/stdio.h lrnref.h /usr/include/signal.h
learn.o: /usr/include/machine/trap.h
list.o: list.c /usr/include/stdio.h lrnref.h /usr/include/signal.h
list.o: /usr/include/machine/trap.h
mem.o: mem.c /usr/include/stdio.h lrnref.h
makpipe.o: makpipe.c /usr/include/stdio.h
maktee.o: maktee.c /usr/include/stdio.h /usr/include/signal.h
maktee.o: /usr/include/machine/trap.h lrnref.h
mysys.o: mysys.c /usr/include/stdio.h /usr/include/signal.h
mysys.o: /usr/include/machine/trap.h
selsub.o: selsub.c /usr/include/stdio.h /usr/include/sys/types.h
selsub.o: /usr/include/sys/stat.h lrnref.h /usr/include/sys/dir.h
selunit.o: selunit.c /usr/include/stdio.h lrnref.h
start.o: start.c /usr/include/stdio.h lrnref.h /usr/include/sys/types.h
start.o: /usr/include/sys/dir.h
whatnow.o: whatnow.c /usr/include/stdio.h lrnref.h
wrapup.o: wrapup.c /usr/include/signal.h /usr/include/machine/trap.h
wrapup.o: /usr/include/stdio.h lrnref.h
lcount.o: lcount.c /usr/include/stdio.h
lrntee.o: lrntee.c

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
