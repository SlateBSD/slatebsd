#	@(#)Makefile	4.1 (Berkeley) 6/2/83
#
# If you don't want to plot, take out the -Dplot and the ref. to plot.a
PLOT = -lplot -Dplot
CFLAGS=-O 
SEPFLAG=-i
prof:	prof.o
	$(CC) $(SEPFLAG) -o prof prof.o $(PLOT)

prof.o: prof.c
	cc -c $(CFLAGS) $(PLOT) prof.c

install:
	install -s prof $(DESTDIR)/usr/bin

clean :
	rm -f *.o prof
