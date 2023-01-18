#	Makefile	4.1	83/08/13
#
DESTDIR=
CFLAGS=	-O
SEPFLAG= -i

calendar: calendar.c
	${CC} ${CFLAGS} ${SEPFLAG} -o calendar calendar.c

install:
	install -s calendar $(DESTDIR)/usr/lib
	install -c -m 755 calendar.sh $(DESTDIR)/usr/bin/calendar

clean:
	rm -f calendar
