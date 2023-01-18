DESTDIR=
BINDIR=	/usr/ucb
LIBDIR=	/usr/lib

CFLAGS= -O -w
LDFLAGS= -X
SEPFLAG= -i

PASCAL= pi px pxp
UTILS= eyacc
ALL= ${PASCAL} ${UTILS}

all: ${PASCAL}

pi pxp: eyacc

${ALL}: FRC
	cd $@; make CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" SEPFLAG=${SEPFLAG} \
		BINDIR=${BINDIR} LIBDIR=${LIBDIR}

install:
	for i in ${PASCAL}; do \
		(cd $$i; make install DESTDIR=${DESTDIR}); \
	done

tags: FRC
	for i in ${ALL}; do \
		(cd $$i; make tags); \
	done

FRC:

clean:
	for i in ${ALL}; do \
		(cd $$i; make clean); \
	done
