#
#	Standalone Makefile
#
#	Note that there are limitations on how large a program may
#	be loaded along with all device drivers.  This is especially
#	a problem with restor.  Programs should be <= 48K to be safe.

DISK=	xp0a
TAPE=	mt8
DUMP=	/usr/root.dump

RB_DEFNAME=xp(0,0)unix

ROOT=	
ETCSRC=	${ROOT}/usr/src/etc
MKFS=	${ETCSRC}/mkfs.c
RESTOR=	${ETCSRC}/restor/restor.c
ICHECK=	${ETCSRC}/icheck.c
USR=	adm bin dict doc games guest hosts include ingres lib local man \
	mdec msgs new old preserve pub skel spool tmp ucb

DEFS=	-DRB_DEFNAME='"${RB_DEFNAME}"' -DSTANDALONE -I${ROOT}/usr/include
CFLAGS=	-O ${DEFS}

BOOT=	M.o boot.o ubmapset.o
DRIVERS=prf.o sys.o \
	ht.o tm.o ts.o \
	xp.o rk.o rl.o rp.o hk.o si.o ra.o

ALL=	mtboot tsboot boot mkfs restor icheck maketape
AUX=	bcopy copy

all: ${ALL}

aux: ${AUX}

mtdist tsdist: ${ALL}
	./maketape /dev/nr${TAPE} tapes/$@
	dump 0f /dev/nr${TAPE} /dev/r${DISK}
	cd ${ROOT}/usr; tar cfb /dev/nr${TAPE} 20 ${USR}

dump:
	dump 0f ${DUMP} /dev/r${DISK}

install: boot
	install -s boot ${ROOT}/


.c.o:
	cc ${CFLAGS} -c $*.c

.s.o:
	as - -V -o $*.o $*.s

libsa.a: ${DRIVERS}
	ar rv $@ $?
	ranlib $@


mkfs.o: ${MKFS}
	cc ${CFLAGS} -c ${MKFS}

restor.o: ${RESTOR}
	cc ${CFLAGS} -c ${RESTOR}

icheck.o: ${ICHECK}
	cc ${CFLAGS} -c ${ICHECK}


mtboot: mtboot.s
	as -o $@ $@.s
	strip $@

tsboot: tsboot.s
	as -o $@ $@.s
	strip $@

boot: M.o c.o boot.o ubmapset.o libsa.a
	ld -X -o $@ M.o c.o boot.o ubmapset.o libsa.a -lc

mkfs: srt0.o c.o libsa.a mkfs.o
	ld -o $@ srt0.o c.o $@.o libsa.a -lc

restor: srt0.o c.o libsa.a restor.o
	ld -o $@ srt0.o c.o $@.o libsa.a -lc

icheck: srt0.o c.o libsa.a icheck.o
	ld -o $@ srt0.o c.o $@.o libsa.a -lc

bcopy: srt0.o c.o libsa.a bcopy.o
	ld -o $@ srt0.o c.o $@.o libsa.a -lc

cat: srt0.o c.o libsa.a cat.o
	ld -o $@ srt0.o c.o $@.o libsa.a -lc

copy: srt0.o c.o libsa.a copy.o
	ld -o $@ srt0.o c.o $@.o libsa.a -lc


maketape: maketape.c
	cc -o $@ maketape.c


tags: FRC
	rm -f tags
	ctags *.[ch]
	for i in M.s; do \
		${EGREP} '^[_a-zA-Z][_a-zA-Z0-9]*:' $$i \
		| ${SED} "s;\([^:]*\):\(.*\)$$;\1	$$i	/^\1:\2$$/;" \
		>> tags; \
	done
	sort -o tags tags

FRC:

clean:
	rm -f ${ALL} ${AUX} libsa.a *.o tags


boot.o:		boot.c
c.o:		c.c
cat.o:		cat.c
hk.o:		hk.c
ht.o:		ht.c
mtboot.o:	mtboot.s
prf.o:		prf.c
rk.o:		rk.c
rl.o:		rl.c
rp.o:		rp.c
srt0.o:		srt0.s
sys.o:		sys.c
tm.o:		tm.c
ts.o:		ts.c
tsboot.o:	tsboot.s
ubmapset.o:	ubmapset.c
xp.o:		xp.c
