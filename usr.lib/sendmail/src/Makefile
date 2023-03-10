#
#  Sendmail
#  Copyright (c) 1983  Eric P. Allman
#  Berkeley, California
#
#  Copyright (c) 1983 Regents of the University of California.
#  All rights reserved.  The Berkeley software License Agreement
#  specifies the terms and conditions for redistribution.
#
#	@(#)Makefile.m4	5.10 (Berkeley) 5/2/86
#
#
#  SENDMAIL Makefile.
#
#

LIBS=	-ldbm
DESTDIR=

OBJS1=	conf.o main.o collect.o parseaddr.o alias.o deliver.o \
	savemail.o err.o readcf.o stab.o headers.o recipient.o \
	stats.o daemon.o usersmtp.o srvrsmtp.o queue.o \
	macro.o util.o clock.o trace.o envelope.o
OBJS2=	sysexits.o arpadate.o convtime.o
OBJS=	$(OBJS1) $(OBJS2)
SRCS1=	conf.h sendmail.h \
	conf.c deliver.c main.c parseaddr.c err.c alias.c savemail.c \
	sysexits.c util.c arpadate.c version.c collect.c \
	macro.c headers.c readcf.c stab.c recipient.c stats.c daemon.c \
	usersmtp.c srvrsmtp.c queue.c clock.c trace.c envelope.c
SRCS2=	TODO convtime.c
SRCS=	Version.c $(SRCS1) $(SRCS2)
ALL=	sendmail

CHOWN=	-echo chown
CHMOD=	chmod
O=	-O
COPTS=
CCONFIG=-I../include -DVMUNIX
CFLAGS=	$O $(COPTS) $(CCONFIG)
SEPFLAG=-i
ASMSED=	../include/asm.sed
AR=	-ar
ARFLAGS=rvu
LINT=	lint
XREF=	ctags -x
CP=	cp
MV=	mv
INSTALL=install -c -s -o root
M4=	m4
TOUCH=	touch
ABORT=	false

GET=	sccs get
DELTA=	sccs delta
WHAT=	what
PRT=	sccs prt
REL=

ROOT=	root
OBJMODE=755

.c.o:
	cc -S ${CFLAGS} $*.c
	sed -f $(ASMSED) $*.s >_xx.s
	as -V - -o $*.o _xx.s
	rm -f $*.s _xx.s

sendmail: $(OBJS1) $(OBJS2) Version.o
	-if [ X$(SEPFLAG) = X-i ]; then \
		$(CC) $(SEPFLAG) $(COPTS) -o sendmail \
			Version.o $(OBJS1) $(OBJS2) $(LIBS); \
	else \
		echo "Need an overlay scheme for non-separate I&D load"; \
	fi
	$(CHMOD) $(OBJMODE) sendmail
	size sendmail; ls -l sendmail; 

install: all
	$(INSTALL) -m 4755 sendmail $(DESTDIR)/usr/lib
	chgrp kmem $(DESTDIR)/usr/lib/sendmail
	$(CP) /dev/null $(DESTDIR)/usr/lib/sendmail.fc

version: newversion $(OBJS) Version.c

newversion:
	@rm -f SCCS/p.version.c version.c
	@$(GET) $(REL) -e SCCS/s.version.c
	@$(DELTA) -s SCCS/s.version.c
	@$(GET) -t -s SCCS/s.version.c

fullversion: $(OBJS) dumpVersion Version.o

dumpVersion:
	rm -f Version.c


$(OBJS1): sendmail.h
$(OBJS): conf.h
stats.o: mailstats.h

sendmail.h util.o: ../include/useful.h

all: $(ALL)

#
#  Auxiliary support entries
#

clean:
	rm -f core sendmail rmail usersmtp uucp a.out XREF sendmail.cf
	rm -f *.o

sources: $(SRCS)


print: $(SRCS)
	@ls -l | pr -h "sendmail directory"
	@$(XREF) *.c | pr -h "cross reference listing"
	@size *.o | pr -h "object code sizes"
	@pr Makefile *.m4 *.h *.[cs]

lint:
	$(LINT) $(CCONFIG) $(SRCS1)
