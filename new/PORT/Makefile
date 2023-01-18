#	Makefile	4.21	86/06/07
#
DESTDIR=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	apl bib cpm help jove kermit mh mkmf patch rcs tac tools
#
# Default programs that are installed
#
STDINST=bib jove mh patch rcs
#
# Special purpose software that requires custom installation,
# or may not be needed by all sites
#
OPTDIR=	B news rn spms sumacc umodem

all:	${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS}

${OPTDIR}: FRC
	cd $@; make ${MFLAGS} all

install:
	-for i in ${STDINST}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done

installall:
	-for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done

clean:
	-for i in ${SUBDIR}; do (echo $$i; cd $$i; make ${MFLAGS} clean); done
	-for i in ${OPTDIR}; do (echo $$i; cd $$i; make ${MFLAGS} clean); done

FRC:
