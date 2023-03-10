/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid: <@(#)execv.s	2.5 (Berkeley) 1/29/87\0>
	.even
#endif SYSLIBC_SCCS

/*
 * XXX - this routine can't use SYSCALL!!!
 */
#include "SYS.h"

.globl	_environ

ENTRY(execv)
	mov	_environ,-(sp)	/ pass default environment,
	mov	6(sp),-(sp)	/   argv list pointer,
	mov	6(sp),-(sp)	/   name
	tst	-(sp)		/ simulate return address stack spacing
	SYS(execve)		/   and go for it ...
	add	$8.,sp		/ if we get back it's an error
	mov	r0,_errno
	mov	$-1,r0
	rts	pc
