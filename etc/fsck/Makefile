#
# fsck makefile
#
CFLAGS=	-O ${FP}
SEPFLAG= -i

SRCS=	fsck.c
OBJS=	fsck.o

fsck: ${OBJS}
	${CC} ${SEPFLAG} -o fsck ${OBJS}

install:
	install -s fsck ${DESTDIR}/etc

clean:
	rm -f *.o a.out core errs fsck

tags:
	cwd=`pwd`; \
	for i in ${SRCS}; do \
		ctags -a -f tags $$cwd/$$i; \
	done
