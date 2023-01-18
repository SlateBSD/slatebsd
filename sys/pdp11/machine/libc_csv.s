/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)csv.s	2.3 (Berkeley) 1/28/87\0>
	.even
#endif LIBC_SCCS

/*
 * C register save and restore routines.  When a C routine is called its stack
 * frame (after csv or ovhandlr have set things up) looks like this:
 *
 *	_________________________________
 *	| return address to caller 	|
 *	|-------------------------------|
 * r5->	| old r5 ("frame pointer")	|
 *	|-------------------------------|
 *	| previous __ovno		|
 *	|-------------------------------|
 *	| old r4			|
 *	|-------------------------------|
 *	| old r3			|
 *	|-------------------------------|
 *	| old r2			|
 *	|-------------------------------|
 * sp->	| empty parameter slot		|
 *	|_______________________________|
 *
 * The "empty parameter slot" is a simple optimization the compiler uses to
 * avoid overhead in its generated parameter cleanup code after a function
 * has been called.  Rather than (for example) generating:
 *
 *	mov	foo,-(sp)	/ push parameter
 *	jsr	pc,bar		/ call function
 *	tst	(sp)+		/ throw away parameter
 *
 * The compiler always keeps one empty slot on the stack and generates:
 *
 *	mov	foo,(sp)	/ pass parameter
 *	jsr	pc,bar		/ call function
 *
 * The savings are not quite so dramatic when more than one parameter is
 * passed, but still worth the effort.  If the function has any local stack
 * variables, space for them will have be be allocated by the function thereby
 * "moving" the empty parameter slot down.
 */
#include "DEFS.h"

#ifdef KERNEL
#include "../machine/mch_iopage.h"
#include "../machine/koverlay.h"
#endif KERNEL

.data
#ifdef KERNEL
.globl	ova, ovd		/ overlay descriptor tables
#endif KERNEL
.globl	__ovno			/ currently mapped overlay
__ovno:	0
.text

.globl	_etext			/ used by cret to determine returns to overlay
				/ area (any return address higher than _etext)

/*
 * The loader (/bin/ld) generates `thunks' for functions loaded in overlays.
 * A thunk resides in the base segment and takes the name _foo which is the
 * function's interface to the outside world (this allows pointers to functions
 * to work even if functions are located in overlays).  The function itself is
 * renamed to ~foo.
 *
 * ovhndlr(1-9,a-f) are called from the thunks via a jsr r5 after r1 is set to
 * the location of the first instruction in the subroutine after the call to
 * csv (~foo+4).
 *
 *	_foo:	mov	$~foo+4,r1
 *		jsr	r5,ovhndlr`n'
 *
 * The ovhndlr`n' in turn move the requested overlay number into r0 and
 * branch to ovhndlr which sets the overlay, simulates a csv and transfers to
 * (r1) (~foo+4).  Thus, the function's call to csv is bypassed.
 */
#define	ovh(x, n)	.globl	ovhndlr/**/x; \
		ovhndlr/**/x: \
			mov	$n,r0; \
			br	ovhndlr;

ovh(1,001);	ovh(2,002);	ovh(3,003);	ovh(4,004)
ovh(5,005);	ovh(6,006);	ovh(7,007);	ovh(8,010)
ovh(9,011);	ovh(a,012);	ovh(b,013);	ovh(c,014)
ovh(d,015);	ovh(e,016);	ovh(f,017)

#ifndef KERNEL
emt	= 0104000		/ overlays switched by emulator trap
				/ overlay number is placed in r0
#endif KERNEL

/*
 * ovhndlr(ov::r0, ~foo+4::r1, _foo+8::r5)
 *
 * Ovhandler builds the new stack frame as fast as possible to avoid problems
 * with getting interrupted halfway through.  if we get interrupted between the
 * "jsr r5,ovhndlr`x'" and the "mov sp,r5" below and a longjmp is attempted,
 * rollback will fail utterly.  If we get interrupted before we finish saving
 * registers reserved for register variables and an attempt is made to longjmp
 * to the caller of our service routine rollback will incorrectly pull `saved'
 * register values from the incomplete stack frame leading to
 * non-deterministic behavior.
 *
 * There's very little that can be done about this asside from changing the
 * entire stack frame sequence which involves changes to the compiler, loader,
 * this file and any other routine (like rollback) which `knows' how the C
 * stack frame is put together ...
 */
ovhndlr:
	mov	sp,r5		/ simulate a csv ...
	mov	__ovno,-(sp)
	cmp	r0,(sp)		/ is the requested overlay already mapped?
	beq	1f
#ifdef KERNEL
	mov	PS,-(sp)	/ save PS
	SPL7
	mov	r0,__ovno	/ set new overlay number
	asl	r0		/ compute descriptor index and map
	mov	ova(r0), OVLY_PAR / the new overlay in
	mov	ovd(r0), OVLY_PDR
	mov	(sp)+,PS	/ restore PS, unmask interrupts
#else !KERNEL
	emt			/   no, ask the kernel for it ...
	mov	r0,__ovno	/     and indicate the new overlay
	/*
	 * SPECIAL NOTE: the pair of instructions "emt" and "mov r0,__ovno"
	 * form a race condition.  A signal can interrupt the sequence
	 * causing all sorts of nastiness if the signal routine calls a
	 * routine in the old overlay or is itself in the old overlay ...
	 * There is no solution that doesn't involve changing the way
	 * an overlay switch from the kernal.  Most [potential] solutions
	 * significantly increase the overhead of an overlay switch.
	 * A tenable solution has yet to be found ...
	 */
#endif KERNEL
1:
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r1)		/ jsr leaves empty parameter slot on stack

/*
 * Csv for routines called directly (in base or intra-overlay calls).
 * no overlays have been changed, so we just save the previous overlay
 * number on the stack.  Note that r0 isn't set to the current overlay
 * because we weren't called through a thunk.
 */
.globl	csv
csv:
	mov	r5,r1		/ save transfer address,
	mov	sp,r5
	mov	__ovno,-(sp)
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r1)

/*
 * Cret is used by everyone so it has to check if we're returning to an
 * overlay different from the one that may be currently mapped.
 */
.globl	cret
cret:
	mov	r5,r2
	mov	-(r2),r4	/ r4 = old __ovno - if non-zero we've started
	bne	2f		/   using overlays so we'll have to make
				/   sure the old overlay is mapped if we're
				/   returning to the overlay area
1:
	mov	-(r2),r4	/ restore registers, reset stack, pop frame
	mov	-(r2),r3	/   pointer and return
	mov	-(r2),r2
	mov	r5,sp		/ (more interrupt problems here *sigh* ...)
	mov	(sp)+,r5
	rts	pc
2:
	/*
	 * Not returning to base segment, so check that the right
	 * overlay is mapped in, and if not change the mapping.
	 */
	cmp	r4,__ovno	/ current overlay same as old overlay?
	beq	1b		/   lucked out!
	cmp	2(r5),$_etext	/ returning to base segment?
	blos	1b		/   sometimes things *do* work out ...
#ifdef KERNEL
	mov	PS,-(sp)	/ (sigh) returning to a different overlay
	SPL7
	mov	r4,__ovno	/ set new overlay number
	asl	r4		/ and map the new overlay in
	mov	ova(r4), OVLY_PAR
	mov	ovd(r4), OVLY_PDR
	mov	(sp)+,PS	/ restore PS, unmask interrupts
	jmp	1b
#else !KERNEL
	mov	r0,r3		/ (sigh) returning to a different overlay -
	mov	r4,r0		/   have to save r0 because we can't trash
	emt			/   a function result and ask UNIX to switch
	mov	r4,__ovno	/   the old overlay in
	mov	r3,r0		/ note that as with ovhndlr the pair "emt"
	br	1b		/   "mov r4,__ovno" can be interrupted
#endif KERNEL
