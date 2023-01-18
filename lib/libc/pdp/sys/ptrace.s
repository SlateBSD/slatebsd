/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid: <@(#)ptrace.s	2.5 (Berkeley) 1/29/87\0>
	.even
#endif SYSLIBC_SCCS

#include "SYS.h"

ENTRY(ptrace)
	clr	_errno		/ -1 is a legitimate return value so we must
	SYS(ptrace)		/   clear errno so the caller may
	bes	error		/   disambiguate
	rts	pc
error:
	mov	r0,_errno
	mov	$-1,r0
	rts	pc
