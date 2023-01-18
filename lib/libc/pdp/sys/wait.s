/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid: <@(#)wait.s	2.5 (Berkeley) 1/29/87\0>
	.even
#endif SYSLIBC_SCCS

/*
 * XXX - this routine can't use SYSCALL!!!
 */
#include "SYS.h"

ENTRY(wait)
	SYS(wait)
	bes	2f
	tst	2(sp)		/ Does the user want the terminated child's
	beq	1f		/   status?
	mov	r1,*2(sp)	/   sure - so give it to them
1:
	rts	pc
2:
	mov	r0,_errno
	mov	$-1,r0
	rts	pc
