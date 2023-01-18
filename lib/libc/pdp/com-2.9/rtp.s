/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid: <@(#)rtp.s	2.5 (Berkeley) 1/29/87\0>
	.even
#endif SYSLIBC_SCCS

#include "SYS.h"

/*
 * C call
 */
SYSCALL(rtp,norm)

/*
 * XXX - this routine can't use SYSCALL!!!
 *
 * F77 call
 */
ENTRY(rtp_)
	mov	*4(sp),-(sp)
	tst	-(sp)		/ simulate return address stack spacing
	SYS(rtp)
	bes	1f
	cmp	(sp)+,(sp)+	/ (clean up stack)
	rts	pc
1:

	cmp	(sp)+,(sp)+	/ (clean up stack)
	mov	r0,_errno
	mov	$-1,r0
	rts	pc
