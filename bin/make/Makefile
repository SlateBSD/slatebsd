# Description file for the Make command
# Makefile 4.4 85/04/16

OBJECTS=ident.o main.o doname.o misc.o files.o dosys.o gram.o 
LIBES= 
LINT=	lint -ps
# 2.10BSD uses the old binary format archives, not the newer ascii format
# CFLAGS=	-O -DASCARCH -I. -I/usr/src/bin/make 
CFLAGS=	-O -I.
SEPFLAG= -i

all:	make

make:	${OBJECTS}
	${CC} -o make ${CFLAGS} ${SEPFLAG} ${OBJECTS} ${LIBES}

${OBJECTS}:  defs

clean:
	-rm -f *.o gram.c make a.out errs

install:
	install -s make ${DESTDIR}/bin/make

lint :  dosys.c doname.c files.c main.c misc.c ident.c gram.c
	${LINT} dosys.c doname.c files.c main.c misc.c ident.c gram.c
	rm -f gram.c
