DESTDIR=

CFLAGS=	-O
SEPFLAG=-i

SRCS=	shar.c traverse.c
OBJS=	shar.o traverse.o

shar: ${OBJS} Makefile
	cc ${CFLAGS} ${SEPFLAG} -o shar ${OBJS}

traverse:
	cc -o traverse -DSTANDALONE traverse.c

lint:
	lint -h ${SRCS}

install: shar shar.1
	install -s shar ${DESTDIR}/usr/new/shar
	install -c shar.1 ${DESTDIR}/usr/man/mann/shar.n

tags: ${SRCS}
	ctags $<

clean:
	rm -f *.o shar traverse core tags
