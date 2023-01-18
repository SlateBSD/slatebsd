#
# vlp makefile
#
CFLAGS=	-O
SEPFLAG= -i

SRCS=	vlp.c
OBJS=	vlp.o

vlp: ${OBJS}
	${CC} -o vlp ${SEPFLAG} ${OBJS}

install:
	install -s vlp ${DESTDIR}/usr/ucb

clean:
	rm -f *.o a.out core errs vlp
