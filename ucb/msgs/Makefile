#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.1 (Berkeley) 6/4/85
#
#
CFLAGS=-O
SEPFLAG= -i
SRCS = msgs.c msgs.h Makefile
msgs: msgs.c msgs.h
	cc ${SEPFLAG} $(CFLAGS) -o msgs msgs.c -ltermlib
install:
	install -s msgs ${DESTDIR}/usr/ucb
clean:
	-rm -f msgs
sources: $(SRCS)
$(SRCS):
	sccs get $@
