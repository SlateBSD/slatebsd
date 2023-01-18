#
# restor makefile
#

# If SEPFLAG is not "-i" then DEFS must include "-DNONSEPARATE" on the
# PDP-11.  Ex:
#	SEPFLAG= -n
#	DEFS= -DNONSEPARATE

SEPFLAG= -i
DEFS= 
CFLAGS=	-O -I. ${DEFS}

SRCS=	restor.c
OBJS=	restor.o

restor: ${OBJS}
	${CC} ${SEPFLAG} ${OBJS} -o restor

install:
	install -s restor ${DESTDIR}/etc

clean:
	rm -f *.o a.out core errs restor

tags:
	cwd=`pwd`; \
	for i in ${SRCS}; do \
		ctags -a -f tags $$cwd/$$i; \
	done
