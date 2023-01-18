/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid: <@(#)fork.s	2.5 (Berkeley) 1/29/87\0>
	.even
#endif SYSLIBC_SCCS

/*
 * XXX - this routine can't use SYSCALL!!!
 */
#include "SYS.h"

.comm	_par_uid,2

ENTRY(fork)
	SYS(fork)
	br	1f			/ child returns here
	bes	2f			/ parent returns here
	rts	pc
1:
	mov	r0,_par_uid
	clr	r0			/ child gets a zero
	rts	pc
2:
	mov	r0,_errno
	mov	$-1,r0
	rts	pc
