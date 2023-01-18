/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)setfpt.s	1.1 (Berkeley) 6/6/87\0>
	.even
#endif LIBC_SCCS

/*
 * Set floating point trap for floating point simulation.  See README for
 * ugly details of reference ordering.  Ask kernel to deliver illegal
 * instructions to fptrap ...
 */
#include "DEFS.h"

.globl	fptrap, _signal

#define	SIGILL	4

#ifdef NONFP
ASENTRY(setfpt)
	mov	$fptrap,-(sp)		/ set floating point trap
	mov	$SIGILL,-(sp)		/   (signal(SIGILL, fptrap))
	jsr	pc,_signal
	cmp	(sp)+,(sp)+
	rts	pc			/ and return
#endif
