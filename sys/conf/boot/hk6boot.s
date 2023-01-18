/*
 *	SCCS id	@(#)hk6boot.s	1.2 (Berkeley)	2/19/87
 */
#include "localopts.h"

/  The boot options and device are placed in the last SZFLAGS bytes
/  at the end of core for the bootstrap.
ENDCORE=	160000		/ end of core, mem. management off
SZFLAGS=	6		/ size of boot flags
BOOTOPTS=	2		/ location of options, bytes below ENDCORE
BOOTDEV=	4
CHECKWORD=	6

reset= 	5

.globl	_doboot, hardboot
.text
_doboot:
	mov	4(sp),r4	/ boot options
	mov	2(sp),r3	/ boot device

#ifndef	KERN_NONSEP
/  If running separate I/D, need to turn off memory management.
/  Call the routine unmap in low text, after setting up a jump
/  in low data where the PC will be pointing.
.globl	unmap
	mov	$137,*$unmap+2		/ jmp *$hardboot
	mov	$hardboot,*$unmap+4
	jmp	unmap
	/ "return" from unmap will be to hardboot in data
.data
#else
/  Reset to turn off memory management
	reset
#endif

/  On power fail, hardboot is the entry point (map is already off)
/  and the args are in r4, r3.

hardboot:
	mov	r4, ENDCORE-BOOTOPTS
	mov	r3, ENDCORE-BOOTDEV
	com	r4		/ if CHECKWORD == ~bootopts, flags are believed
	mov	r4, ENDCORE-CHECKWORD
1:
	reset

/  The remainder of the code is dependent on the boot device.
/  If you have a bootstrap ROM, just jump to the correct entry.
/  Otherwise, use a BOOT opcode, if available;
/  if necessary, read in block 0 to location 0 "by hand".

/ rk06 disk driver

WC = -256.

hkcs1 = 177440	/ control & status 1
hkda  = 177446	/ desired track/sector address
hkcs2 = 177450	/ control & status 2
hkca  = 177460	/ desired cylinder

/ RK06 constants.
ack = 03	/ pack acknowledge
clear = 040	/ subsystem clear
iocom = 021	/ read + go

/ initialize hk
	mov	$clear,hkcs2
	mov	$ack,hkcs1
0:
	tstb	hkcs1
	bpl	0b		/ wait for acknowledge to complete

	clr	hkca
	mov	$hkda,r1
	clr	(r1)		/ sector and track
	clr	-(r1)		/ bus address
	mov	$WC,-(r1)	/ word count
	mov	$iocom,-(r1)
1:
	tstb	(r1)
	bge	1b		/ wait for iocom to complete
	jmp	*$0
