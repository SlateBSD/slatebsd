#
#	Makefile	4.4	86/06/07
#
DESTDIR= 
CFLAGS=	-O
LDFLAGS=
SEPFLAG= -i
CFILES=	access.c command.c dummy.c expr.c findfn.c format.c input.c \
	main.c message.c opset.c output.c pcs.c print.c runpcs.c \
	setup.c sym.c
OV1=	input.o main.o command.o setup.o expr.o
OV2=	format.o print.o findfn.o
OV3=	output.o opset.o pcs.o
BASE=	access.o sym.o runpcs.o message.o
OFILES=	${OV1} ${OV2} ${OV3} ${BASE}

adb: ${OFILES}
	-if [ X${SEPFLAG} = X-i ]; then \
		${CC} ${SEPFLAG} ${LDFLAGS} -o adb ${OFILES}; \
	else \
		ld -X ${SEPFLAG} ${LDFLAGS} -o adb /lib/crt0.o \
		-Z ${OV1} -Z ${OV2} -Z ${OV3} -Y ${BASE} -lc; \
	fi

install:
	install -s adb ${DESTDIR}/bin

clean:
	rm -f adb instrs.adb errs ${OFILES}

print:
	@ls -l | pr
	@pr -f Makefile *.h ${CFILES}

defs.h:		mac.h mode.h
mode.h:		machine.h

access.o:	defs.h access.c
command.o:	defs.h command.c
expr.o:		defs.h expr.c
findfn.o:	defs.h findfn.c
findrtn.o:	defs.h findrtn.c
format.o:	defs.h format.c
input.o:	defs.h mode.h input.c
main.o:		defs.h main.c
message.o:	defs.h message.c
opset.o:	defs.h opset.c
output.o:	defs.h output.c
pcs.o:		defs.h pcs.c
print.o:	defs.h print.c
runpcs.o:	defs.h mode.h runpcs.c
setup.o:	defs.h setup.c
sym.o:		defs.h sym.c
