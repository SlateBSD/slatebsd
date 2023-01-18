#
# window makefile
#
CFLAGS=	-O
SEPFLAG= -i

SRCS=	window.c
OBJS=	window.o

window: ${OBJS}
	${CC} -o window ${SEPFLAG} ${OBJS}

install:
	install -s window ${DESTDIR}/usr/ucb

clean:
	rm -f *.o a.out core errs window
