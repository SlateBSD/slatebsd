#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.4	(Berkeley)	7/22/87
#
CFLAGS=	-O
LIBC=	/lib/libc.a
SRCS=	cmds.c cmdtab.c ftp.c getpass.c glob.c main.c pclose.c ruserpass.c              domacro.c
OBJS=	cmds.o cmdtab.o ftp.o getpass.o glob.o main.o pclose.o ruserpass.o              domacro.o

all: ftp

ftp:	${OBJS} ${LIBC}
	${CC} ${SEPFLAG} -o $@ ${CFLAGS} ${OBJS}

clean: FRC
	rm -f ${OBJS} core ftp

depend: FRC
	mkdep ${CFLAGS} ${SRCS}

install: FRC
	install -s -m 755 ftp ${DESTDIR}/usr/ucb/ftp

lint: FRC
	lint ${CFLAGS} ${SRCS}

tags: FRC
	ctags ${SRCS}

FRC:

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

cmds.o: cmds.c ftp_var.h /usr/include/sys/param.h /usr/include/sys/types.h
cmds.o: /usr/include/signal.h /usr/include/machine/trap.h
cmds.o: /usr/include/machine/machparam.h /usr/include/setjmp.h
cmds.o: /usr/include/sys/socket.h /usr/include/arpa/ftp.h /usr/include/signal.h
cmds.o: /usr/include/stdio.h /usr/include/errno.h /usr/include/netdb.h
cmds.o: /usr/include/ctype.h /usr/include/sys/wait.h
cmdtab.o: cmdtab.c ftp_var.h /usr/include/sys/param.h /usr/include/sys/types.h
cmdtab.o: /usr/include/signal.h /usr/include/machine/trap.h
cmdtab.o: /usr/include/machine/machparam.h /usr/include/setjmp.h
ftp.o: ftp.c ftp_var.h /usr/include/sys/param.h /usr/include/sys/types.h
ftp.o: /usr/include/signal.h /usr/include/machine/trap.h
ftp.o: /usr/include/machine/machparam.h /usr/include/setjmp.h
ftp.o: /usr/include/sys/stat.h /usr/include/sys/ioctl.h
ftp.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
ftp.o: /usr/include/sys/socket.h /usr/include/sys/time.h /usr/include/time.h
ftp.o: /usr/include/sys/param.h /usr/include/sys/types.h /usr/include/signal.h
ftp.o: /usr/include/machine/machparam.h /usr/include/netinet/in.h
ftp.o: /usr/include/arpa/ftp.h /usr/include/arpa/telnet.h /usr/include/stdio.h
ftp.o: /usr/include/signal.h /usr/include/errno.h /usr/include/netdb.h
ftp.o: /usr/include/fcntl.h /usr/include/pwd.h /usr/include/ctype.h
getpass.o: getpass.c /usr/include/stdio.h /usr/include/signal.h
getpass.o: /usr/include/machine/trap.h /usr/include/sgtty.h
getpass.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
getpass.o: /usr/include/sys/ttydev.h
glob.o: glob.c /usr/include/sys/param.h /usr/include/sys/types.h
glob.o: /usr/include/signal.h /usr/include/machine/trap.h
glob.o: /usr/include/machine/machparam.h /usr/include/sys/stat.h
glob.o: /usr/include/sys/dir.h /usr/include/stdio.h /usr/include/errno.h
glob.o: /usr/include/pwd.h
main.o: main.c ftp_var.h /usr/include/sys/param.h /usr/include/sys/types.h
main.o: /usr/include/signal.h /usr/include/machine/trap.h
main.o: /usr/include/machine/machparam.h /usr/include/setjmp.h
main.o: /usr/include/sys/socket.h /usr/include/sys/ioctl.h
main.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
main.o: /usr/include/sys/types.h /usr/include/arpa/ftp.h /usr/include/signal.h
main.o: /usr/include/stdio.h /usr/include/errno.h /usr/include/ctype.h
main.o: /usr/include/netdb.h /usr/include/pwd.h
pclose.o: pclose.c /usr/include/stdio.h /usr/include/signal.h
pclose.o: /usr/include/machine/trap.h /usr/include/sys/param.h
pclose.o: /usr/include/sys/types.h /usr/include/signal.h
pclose.o: /usr/include/machine/machparam.h /usr/include/sys/wait.h
ruserpass.o: ruserpass.c /usr/include/stdio.h /usr/include/utmp.h
ruserpass.o: /usr/include/ctype.h /usr/include/sys/types.h
ruserpass.o: /usr/include/sys/stat.h /usr/include/errno.h
domacro.o: domacro.c ftp_var.h /usr/include/sys/param.h
domacro.o: /usr/include/sys/types.h /usr/include/signal.h
domacro.o: /usr/include/machine/trap.h /usr/include/machine/machparam.h
domacro.o: /usr/include/setjmp.h /usr/include/signal.h /usr/include/stdio.h
domacro.o: /usr/include/errno.h /usr/include/ctype.h
domacro.o: /usr/include/sys/ttychars.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
