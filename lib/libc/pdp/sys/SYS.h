/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)SYS.h	1.3 (Berkeley) 1/7/87
 */

#include <syscall.h>

.comm	_errno,2

#define	ENTRY(x)	.globl _/**/x; \
		_/**/x: \
			PROFCODE(_/**/x);

#define	ASENTRY(x)	.globl x; \
		x: \
			PROFCODE(x);

#ifdef PROF
#define	PROFCODE(x)	.data; \
		1:	x+1; \
			.text; \
			.globl	mcount; \
			mov	$1b, r0; \
			jsr	pc,mcount;
#else !PROF
#define	PROFCODE(x)	;
#endif PROF

#define	SYS(s)		sys	SYS_/**/s.

#define	SYSCALL(s, r)	ENTRY(s); \
			SYS(s); \
			EXIT_/**/r

#define	PSEUDO(f, s, r)	ENTRY(f); \
			SYS(s); \
			EXIT_/**/r


#define	EXIT_norm		bes	error; \
				rts	pc; \
			error: \
				mov	r0,_errno; \
				mov	$-1,r0; \
				rts	pc;

#define	EXIT_long		bes	error; \
				rts	pc; \
			error: \
				mov	r0,_errno; \
				mov	$-1,r1; \
				sxt	r0; \
				rts	pc;

#define	EXIT_error		mov	r0,_errno; \
				mov	$-1,r0; \
				rts	pc;

#define	EXIT_noerror		rts	pc;

#define	EXIT_alt_noerror	mov	r1,r0; \
				rts	pc;
