/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)fgetc.s	1.1 (Berkeley) 2/4/87\0>
	.even
#endif LIBC_SCCS

#include "DEFS.h"
#include "STDIO.h"

.globl	__filbuf

/*
 * fgetc(iop)
 *	FILE	*iop;
 *
 * Return next character from stream fp or EOF on end of file.
 *
 */
ENTRY(fgetc)
	mov	2(sp),r1		/ grab iop
	dec	_CNT(r1)		/ any characters available?
	blt	1f
	movb	*_PTR(r1),r0		/ grab the character,
	inc	_PTR(r1)		/ bop the pointer on one place,
	bic	$!377,r0		/ and cast to u_char
	rts	pc
1:
	jmp	__filbuf		/ let _filbuf(iop) handle it
