/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mch_start.s	1.1 (2.10BSD Berkeley) 2/10/87
 */

#include "DEFS.h"
#include "../machine/mch_iopage.h"
#include "../machine/mch_cpu.h"
#include "../machine/trap.h"


ASENTRY(start)
	bit	$1,SSR0			/ is memory management enabled?
	beq	.			/ better be !!!

	/*
	 * The following two instructions change the contents of the "sys"
	 * instruction vector (034-037) to read:
	 *	syscall; br0+T_SYSCALL
	 */
	mov	$syscall,34
	mov	$0+T_SYSCALL,36

#ifdef KERN_NONSEP
	movb	$RO,KISD0		/ Turn off write permission on kernel
					/   low text
#endif

	mov	$USIZE-1\<8|RW,KDSD6	/ Get a stack pointer (_u + 64*USIZE)
	mov	$_u+[USIZE*64.],sp

	mov	$_u,r0			/ Clear user block
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b

	/*
	 * Get bootflags and leave them in _initflags; the bootstrap leaves
 	 * them in r4.  R2 should be the complement of bootflags.
	 */
	com	r2			/ if r4 != ~r2
	cmp	r4,r2
	beq	1f
	mov	$RB_SINGLE,r4		/   r4 = RB_SINGLE
1:
	mov	r4,_boothowto		/ save boot flags
	mov	$_initflags+6,r2	/ get a pointer to the \0 in _initflags
	mov	r4,r1			/ r1 = boot options
1:
	clr	r0			/ r0:r1 = (long)r1
	div	$10.,r0			/ r0 = r0:r1 / 10; r1 = r0:r1 % 10
	add	$'0,r1			/ bias by ASCII '0'
	movb	r1,-(r2)		/   and stuff into _initflags
	mov	r0,r1			/ shift quotient and continue
	bne	1b			/   if non-zero

	/* Check out (and if necessary, set up) the hardware. */
	jsr	pc,hardprobe

	/*
	 * Set up previous mode and call main.  On return, set user stack
	 * pointer and enter user mode at location zero to execute the
	 * init thunk _icode copied out in main.
	 */
	mov	$30340,PS		/ set current kernel mode, previous
					/   mode to user, spl7, make adb
	clr	r5			/   find end of kernel stack frames
	jsr	pc,_main		/ call main
	mov	$177776,-(sp)		/ set init thunk stack pointer to top
	mtpd	sp			/   of memory ...
	mov	$170000,-(sp)		/ set up pseudo trap frame: user/user
	clr	-(sp)			/   mode, location zero,
	rtt				/   and book ...


.data
/*
 * Icode is copied out to process 1 to exec /etc/init.
 * If the exec fails, process 1 exits.
 */
.globl	_initflags, _szicode, _boothowto

ENTRY(icode)
	mov	$argv-_icode,-(sp)
	mov	$init-_icode,-(sp)
	tst	-(sp)			/ simulate return address stack spacing
	sys	SYS_execv
	sys	SYS_exit
init:
	</etc/init\0>
_initflags:
	<-00000\0>			/ decimal ASCII initflags
	.even
argv:
	init+5-_icode			/ address of "init\0"
	_initflags-_icode
	0
_szicode:
	_szicode-_icode
_boothowto:
	0				/ boot flags passed by boot
.text


/*
 * Check out (and if necessary, set up) the hardware.  A lot of the work done
 * here is to figure out what we've really got.  In many cases even if a
 * certain capability is defined (separate I/D, etc.), we check to see if it's
 * really there.  This allows us to distribute a generic kernel that will run
 * on any processor but still take advantage of hardware that is present.
 * This is also a plus for a site which wants to maintain one kernel for a
 * number of different processors.
 */
hardprobe:
#ifndef NONFP
	mov	$1f,nofault
	setd
	inc	fpp
1:
#endif

	/*
	 * Test for SSR3 and UNIBUS map capability.  If there is no SSR3, the
	 * first test of SSR3 will trap and we skip past the separate I/D test.
	 */
	mov	$cputest,nofault
#ifdef UNIBUS_MAP
	bit	$40,SSR3
	beq	1f
	incb	_ubmap
1:
#endif

#ifdef NONSEPARATE
	/*
	 * Don't attempt to determine whether we've got separate I/D
	 * (but just in case we do, we must force user unseparated
	 * because boot will have turned on separation if possible).
	 */
	bic	$1,SSR3
#else
	bit	$1,SSR3			/ Test for separate I/D capability
	beq	cputest
	incb	_sep_id
#endif

	/*
	 * Try to find out what kind of cpu this is.  Defaults are 40 for
	 * nonseparate and 45 for separate.  Cputype will be one of: 24,
	 * 40, 60, 45, 44, 70, 73.
	 */
cputest:
#ifndef NONSEPARATE
	tstb	_sep_id
	beq	nonsepcpu

	tstb	_ubmap			/ sep_id: 44, 45, 70, 73
	bne	1f

	mov	$cpudone,nofault	/ sep_id && !ubmap: 45 or 73
	tst	*$PDP1170_MSER		/ mem sys err reg implies 73
	mov	$73.,_cputype
	bis	$CCR_DT,*$PDP1170_CCR	/ disable cache traps
	br	cpudone
1:
	mov	$1f,nofault		/ sep_id && ubmap: 44 or 70
	mfpt				/ if mfpt instruction exists, this is
	mov	$44.,_cputype		/   a 44
	bis	$CCR_DCPI,*$PDP1144_CCR	/ Disable cache parity interrupts.
	br	cpudone
1:
	mov	$70.,_cputype
	bis	$CCR_DUT|CCR_DT,*$PDP1170_CCR / Disable UNIBUS and nonfatal
	br	cpudone			/   traps.

nonsepcpu:
#endif !NONSEPARATE
	tstb	_ubmap			/ !sep_id: 24, 40, 60
	bne	1f

	mov	$cpudone,nofault	/ !sep_id && !ubmap: 40, 60
	tst	PDP1160_MSR
	mov	$60.,_cputype
	bis	$CCR_DT,*$PDP1160_CCR	/ Disable cache parity error traps.
	br	cpudone
1:
	mov	$24.,_cputype		/ !sepid && ubmap: 24

cpudone:
		/ Test for stack limit register; set it if present.
	mov	$1f,nofault
	mov	$intstk-256.,STACKLIM
1:

#ifdef ENABLE34
	/*
	 * Test for an ENABLE/34.  We are very cautious since the ENABLE's
	 * PARs are in the range of the floating addresses.
	 */
	tstb	_ubmap
	bne	2f
	mov	$2f,nofault
	mov	$32.,r0
	mov	$ENABLE_KISA0,r1
1:
	tst	(r1)+
	sob	r0,1b

	tst	*$PDP1170_LEAR
	tst	*$ENABLE_SSR3
	tst	*$ENABLE_SSR4
	incb	_enable34
	incb	_ubmap

	/*
	 * Turn on an ENABLE/34.  Enableon() is a C routine which does a PAR
	 * shuffle and turns mapping on.
	 */

.data
_UISA:	DEC_UISA
_UDSA:	DEC_UDSA
_KISA0:	DEC_KISA0
_KISA6:	DEC_KISA6
_KDSA1:	DEC_KDSA1
_KDSA2:	DEC_KDSA2
_KDSA5:	DEC_KDSA5
_KDSA6:	DEC_KDSA6

.text
	mov	$ENABLE_UISA, _UISA
	mov	$ENABLE_UDSA, _UDSA
	mov	$ENABLE_KISA0, _KISA0
	mov	$ENABLE_KISA6, _KISA6
	mov	$ENABLE_KDSA1, _KDSA1
	mov	$ENABLE_KDSA2, _KDSA2
	mov	$ENABLE_KDSA5, _KDSA5
	mov	$ENABLE_KDSA6, _KDSA6
	mov	$ENABLE_KDSA6, _ka6
	jsr	pc, _enableon

2:
#endif ENABLE34

	clr	nofault			/ clear fault trap and return
	rts	pc
