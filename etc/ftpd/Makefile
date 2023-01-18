#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.4	(Berkeley)	6/2/87
#
CFLAGS=	-O
LIBC=	/lib/libc.a
SRCS=	ftpd.c ftpcmd.c glob.c vers.c
OBJS=	ftpd.o ftpcmd.o glob.o vers.o

all: ftpd

ftpd: ${OBJS} ${LIBC}
	${CC} ${SEPFLAG} -o $@ ${OBJS}

vers.o: ftpd.c ftpcmd.y
	sh newvers.sh
	${CC} ${CFLAGS} -c vers.c

clean: FRC
	rm -f ${OBJS} ftpd core ftpcmd.c

depend: ftpcmd.c FRC
	mkdep ${CFLAGS} ${SRCS}

install: FRC
	install -s -m 755 ftpd ${DESTDIR}/etc/ftpd

lint: ftpcmd.c FRC
	lint ${CFLAGS} ${SRCS}

tags: ftpcmd.c FRC
	ctags ${SRCS}

FRC:

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

ftpd.o: ftpd.c /usr/include/sys/param.h /usr/include/sys/types.h
ftpd.o: /usr/include/signal.h /usr/include/machine/trap.h
ftpd.o: /usr/include/machine/machparam.h /usr/include/sys/stat.h
ftpd.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
ftpd.o: /usr/include/sys/ttydev.h /usr/include/sys/socket.h
ftpd.o: /usr/include/sys/file.h /usr/include/sys/wait.h
ftpd.o: /usr/include/netinet/in.h /usr/include/arpa/ftp.h
ftpd.o: /usr/include/arpa/inet.h /usr/include/arpa/telnet.h
ftpd.o: /usr/include/stdio.h /usr/include/signal.h /usr/include/pwd.h
ftpd.o: /usr/include/setjmp.h /usr/include/netdb.h /usr/include/errno.h
ftpd.o: /usr/include/strings.h /usr/include/syslog.h /usr/include/utmp.h
ftpcmd.o: ftpcmd.c /usr/include/sys/types.h /usr/include/sys/socket.h
ftpcmd.o: /usr/include/netinet/in.h /usr/include/arpa/ftp.h
ftpcmd.o: /usr/include/stdio.h /usr/include/signal.h
ftpcmd.o: /usr/include/machine/trap.h /usr/include/ctype.h /usr/include/pwd.h
ftpcmd.o: /usr/include/setjmp.h /usr/include/syslog.h
ftpcmd.o: /usr/include/arpa/telnet.h
glob.o: glob.c /usr/include/sys/param.h /usr/include/sys/types.h
glob.o: /usr/include/signal.h /usr/include/machine/trap.h
glob.o: /usr/include/machine/machparam.h /usr/include/sys/stat.h
glob.o: /usr/include/sys/dir.h /usr/include/stdio.h /usr/include/errno.h
glob.o: /usr/include/pwd.h
vers.o: vers.c

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
