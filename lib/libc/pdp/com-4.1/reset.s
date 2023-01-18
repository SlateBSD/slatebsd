/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#ifdef LIBC_SCCS
	<@(#)reset.c	1.1 (Berkeley) 1/25/87\0>
	.even
#endif LIBC_SCCS

/*
 * val = setexit()
 *	int	val;
 *
 * reset(val)
 *	int	val;
 *
 * reset(val) will generate a "return(val)" from the last call to setexit() by
 * restoring the general registers r2-r4 (register variables), r5 (fp), sp and
 * pc from static storage as saved by setexit.  See setjmp(3) for a more
 * general interface.
 */
#include "DEFS.h"

.data
setsav:	. = . + [10*4]
.text

ENTRY(setexit)
	mov	$setsav,r0	/ set up to load save area
	mov	r2,(r0)+	/ save register variables,
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+	/   the frame pointer,
	mov	sp,(r0)+	/   the current stack pointer,
	mov	(sp),(r0)+	/   and the return pc
	clr	r0		/ and return a 0
	rts	pc

ENTRY(reset)
	mov	2(sp),r0	/ grab the return value
	mov	$setsav,r1	/ and reload all the old values
	mov	(r1)+,r2	/   register variables,
	mov	(r1)+,r3
	mov	(r1)+,r4
	mov	(r1)+,r5	/   frame pointer,
	mov	(r1)+,sp	/   and stack pointer
	mov	(r1),(sp)	/ put old return address on top of stack
	rts	pc		/   and return ...
