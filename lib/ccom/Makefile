CFLAGS = -O -n
LIB = /lib

all: c0 c1 cvopt
c0: c00.o c01.o c02.o c03.o c04.o c05.o
	${CC} ${CFLAGS} -o c0 c00.o c01.o c02.o c03.o c04.o c05.o

c00.o c01.o c02.o c03.o c04.o c05.o: c0.h

c1: c10.o c11.o c12.o c13.o table.o
	${CC} ${CFLAGS} -o c1 c10.o c11.o c12.o c13.o table.o

c10.o c11.o c12.o c13.o: c1.h

table.o: optable cvopt
	./cvopt < optable > junk.c
	${CC} -S junk.c
	sed 's/\.data/\.text/' < junk.s > junk.i
	as -o table.o junk.i
	rm -f junk.i junk.c junk.s

cvopt:	cvopt.c
	${CC} -n -s -O -o cvopt cvopt.c

install: c0 c1
	-mv ${LIB}/c0 ${LIB}/oc0
	-mv ${LIB}/c1 ${LIB}/oc1
	install -s c0 ${LIB}
	install -s c1 ${LIB}

restore:
	mv ${LIB}/oc0 ${LIB}/c0
	mv ${LIB}/oc1 ${LIB}/c1

clean:
	rm -f *.o c0 c1 cvopt
