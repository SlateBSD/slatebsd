DESTDIR = /usr/games
LIBDIR  = /usr/games/lib
CFLAGS  = -O
SEPFLAG	= -i

OBJ=	agen.o att.o bgen.o bheur.o bmove.o book.o bplay.o ctrl.o data.o\
	init.o io.o mater.o play.o qsort.o savres.o setup.o stat.o\
	stdin.o wgen.o wheur.o wmove.o wplay.o

chess:	${OBJ}
	cc ${SEPFLAG} -o chess ${OBJ}

init.o:
	cc -c -O -DBOOK=\"${LIBDIR}/book\" init.c

${OBJ}:	old.h

install: chess
	install -s chess ${DESTDIR}
#	install -c -m 644 book ${LIBDIR}/book

clean:
	rm -f *.o a.out chess core
