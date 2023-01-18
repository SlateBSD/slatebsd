#
# Copyright (c) 1980, 1986 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	7.1 (Berkeley) 6/5/86
#
ALL=	dvhpuboot hkuboot hpuboot rauboot rkuboot rluboot rm03uboot \
	rm05uboot rpuboot si94uboot si95uboot

all: ${ALL}

${ALL}:
	/lib/cpp -E $*.s | sed -e 's;^#;/;' > x.s
	as x.s
	rm x.s
	nm -u a.out
	size a.out
	strip a.out
	dd if=a.out of=b.out bs=16 skip=1
	dd if=b.out of=$@ conv=sync

dvhpuboot:	dvhpuboot.s
hkuboot:	hkuboot.s
hpuboot:	hpuboot.s
rauboot:	rauboot.s
rkuboot:	rkuboot.s
rluboot:	rluboot.s
rm03uboot:	rm03uboot.s
rm05uboot:	rm05uboot.s
rpuboot:	rpuboot.s
si94uboot:	si94uboot.s
si95uboot:	si95uboot.s

clean:
	rm -f x.s a.out b.out ${ALL}

install:
	cp *boot ${DESTDIR}/mdec
