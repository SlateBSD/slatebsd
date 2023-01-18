/*
 *	SCCS id	@(#)rmboot.s	1.2 (Berkeley)	2/19/87
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

/ rm02/3/5 bootstrap - salkind@nyu

WC	= -256.
READ	= 70
GO	= 1
PRESET	= 20
FMT22	= 10000
DRIVE	= 0

rmcs1	= 176700
rmda	= rmcs1+6
rmcs2	= rmcs1+10
rmds	= rmcs1+12
rmof	= rmcs1+32
rmca	= rmcs1+34

	mov	$DRIVE,*$rmcs2
	mov	$PRESET+GO,*$rmcs1
	mov	$FMT22,*$rmof
	clr	*$rmca
	mov	$rmda,r0
	clr	(r0)
	clr	-(r0)
	mov	$WC,-(r0)
	mov	$READ+GO,-(r0)
1:
	tstb	(r0)
	bge	1b
	jmp	*$0
