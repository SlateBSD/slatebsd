#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.14 (Berkeley) 5/16/86
#
DESTDIR=
CFLAGS=	-O
SEPFLAG= -i

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	config dump fsck ftpd getty htable implog named restor \
	routed talkd termcap tftpd timed tzone XNSrouted

# Shell scripts that need only be installed and are never removed.
#
SCRIPT=	mklost

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	ac accton arff bad144 badsect catman chown clri comsat \
	cron dcheck diskpart dumpfs edquota fingerd flcopy \
	gettable ifconfig icheck inetd init newfs mkfs mknod \
	mkproto mount ncheck quotaon quotacheck reboot renice \
	repquota rexecd rlogind rmt route rshd rwhod rxformat sa savecore \
	shutdown slattach swapon syslogd telnetd tunefs umount update vipw \
	mkpasswd

# C programs that live in the current directory and need explicit make lines.
#
NSTD=

# Programs that must run setuid to root
#
SETUID=	ping

# C programs that must run set-group-id kmem.
#
#KMEM=	pstat arp dmesg kgmon trpt
KMEM=	arp dmesg kgmon trpt

# C programs that must run set-group-id operator.
#
OPERATOR= quot

# optional: not always made
OPTIONAL= mkhosts

all:	${SUBDIR} ${STD} ${NSTD} ${OPTIONAL} ${KMEM} ${SETUID} ${OPERATOR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS} SEPFLAG=${SEPFLAG}

${STD} ${KMEM} ${SETUID} ${OPERATOR}:
	cc ${CFLAGS} ${SEPFLAG} -o $@ $@.c

install:
	-for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done
	-for i in ${SCRIPT}; do (install -c $$i.sh ${DESTDIR}/etc/$$i); done
	install -s ${STD} ${NSTD} ${DESTDIR}/etc
	-for i in ${OPTIONAL}; do \
		(if [ -f $$i ]; then install -s $$i ${DESTDIR}/etc; fi); done
	-for i in ${SETUID}; do \
		(install -o root -m 4755 -s $$i ${DESTDIR}/etc/$$i); done
	-for i in ${KMEM}; do \
		(install -g kmem -m 2755 -s $$i ${DESTDIR}/etc/$$i); done
	-for i in ${OPERATOR}; do \
		(install -g operator -m 2755 -s $$i ${DESTDIR}/etc/$$i); done
	-chown root.operator ${DESTDIR}/etc/shutdown
	-chmod 4750 ${DESTDIR}/etc/shutdown
	-rm -f ${DESTDIR}/etc/quotaoff
	-ln ${DESTDIR}/etc/quotaon ${DESTDIR}/etc/quotaoff
	-rm -f ${DESTDIR}/etc/fastboot ${DESTDIR}/etc/halt
	-ln ${DESTDIR}/etc/reboot ${DESTDIR}/etc/fastboot
	-ln ${DESTDIR}/etc/reboot ${DESTDIR}/etc/halt
	-mkdir ${DESTDIR}/usr/spool/rwho
	-chmod 700 ${DESTDIR}/etc/init

clean:
	-rm -f a.out core *.s *.o
	-for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done
	-rm -f ${STD} ${NSTD} ${KMEM} ${SETUID}

FRC:

depend:
	for i in ${STD} ${NSTD} ${KMEM} ${SETUID}; do \
	    cc -M ${INCPATH} $$i.c | sed 's/\.o//' | \
	    awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
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

# Files listed in ${NSTD} and ${OPTIONAL} have explicit make lines given below.

mkhosts:
	@echo "Mkhosts only works with the dbm version of the hostent routines."
	@echo "If gethosts() is not in libc, mkhosts isn't installed."
	-cc ${CFLAGS} -o mkhosts mkhosts.c || rm mkhosts

# DO NOT DELETE THIS LINE -- make depend uses it

