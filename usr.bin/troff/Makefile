#
#	@(#)Makefile	1.4 3/30/83
#
DESTDIR =
SEPFLAG= -i
SRCS = \
	ni.c nii.c n1.c n2.c \
	n3.c n4.c n5.c t6.c \
	n7.c n8.c n9.c t10.c \
	hytab.c ntab.c suftab.c \
	d.h tw.h v.h tdef.h sdef.h

# Use the following flag set for 4.3BSD
# CFLAGS = -O -DVMUNIX

# Use the following flag set for 2.10BSD
CFLAGS = -O

c = ni.o nii.o n1.o n2.o n3.o n4.o n5.o t6.o n7.o n8.o n9.o t10.o
s = hytab.o suftab.o tab3.o

troff: $c $s
	$(CC) ${SEPFLAG} -o troff $s $c

hytab.o: hytab.c
	$(CC) -c hytab.c

suftab.o: suftab.c
	$(CC) -c suftab.c

install: troff
	install -s troff ${DESTDIR}/usr/bin

clean:
	rm -f *.o troff
