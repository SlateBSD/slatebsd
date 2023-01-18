#
# makefile for as
#

AS=	/bin/as
ASFLAGS= - -V

AS1SRC = as11.s as12.s as13.s as14.s as15.s as16.s as17.s as18.s as19.s
AS2SRC = as21.s as22.s as23.s as24.s as25.s as26.s as27.s as28.s as29.s

all: as as2

#
# "as" can't be loaded with either -i or -n
#
as: ${AS1SRC}
	${AS} ${ASFLAGS} -o as1.o as1?.s
	ld -o $@ as1.o -lc

as2: ${AS2SRC}
	${AS} ${ASFLAGS} -o as2.o as2?.s
	ld -o $@ as2.o -lc

install:
	install -s as ${DESTDIR}/bin
	install -s as2 ${DESTDIR}/lib

clean:
	rm -f *.o a.out as as2 core errs
