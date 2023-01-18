/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)setjmp.s	1.3 (Berkeley) 1/6/87\0>
	.even
#endif LIBC_SCCS

/*
 * val = setjmp(env)
 *	int	val;
 *	jmp_buf	*env;
 *
 * longjmp(env, val)
 *	jmp_buf	*env;
 *	int	val;
 *
 * Overlaid version of setjmp and longjmp.  MUST BE LOADED IN BASE SEGMENT!!!
 *
 * Longjmp(env,val) will generate a "return(val)" from the last call to
 * setjmp(env) by restoring the general registers r2, r3, and r4 from the
 * stack and performing a sigreturn on the sigcontext constructed in env by
 * setjmp, see <signal.h>.
 */
#include "DEFS.h"

.globl	_sigstack, _sigblock	/ needed to create sigcontext
.globl	__ovno

ENTRY(setjmp)
	mov	r2,-(sp)	/ save r2
	mov	4(sp),r2	/ r2 = env
	sub	$4.,sp		/ allocate sizeof(struct sigstack)
	mov	sp,r0		/   and get current sigstack via
	mov	r0,-(sp)	/   sigstack(0, sp) (can't use "mov sp,-(sp)")
	clr	-(sp)
	jsr	pc,_sigstack
	add	$6.,sp		/ toss signal stack value and
	mov	(sp)+,(r2)+	/   save onsigstack status of caller
	clr	-(sp)		/ get current signal mask via
	clr	-(sp)		/   sigblock(0L)
	jsr	pc,_sigblock
	cmp	(sp)+,(sp)+
	mov	r0,(r2)+	/ save signal mask of caller
	mov	r1,(r2)+
	mov	sp,(r2)		/ calculate caller's pre jsr pc,setjmp
	add	$4,(r2)+	/   sp as (sp + saved r2 + ret addr)
	mov	r5,(r2)+	/ save caller's frame pointer
	cmp	(r2)+,(r2)+	/   fake r0 & r1
	mov	2(sp),(r2)+	/   return pc,
	mov	$0170000,(r2)+	/   fake (but appropriate) ps, and
	mov	__ovno,(r2)	/   current overlay
	mov	(sp)+,r2	/ restore r2
	clr	r0		/ and return a zero
	rts	pc

SC_FP	= 8.			/ offset of sc_fp in sigcontext
SC_R0	= 12.			/ offset of sc_r0 in sigcontext
iot	= 4

.globl	rollback, _sigreturn, __ljerr
ENTRY(longjmp)
	mov	2(sp),r1	/ r1 = env
	mov	SC_FP(r1),r0	/ r0 = env->sc_fp
	jsr	pc,rollback	/ attempt stack frame rollback
	mov	4(sp),r0	/ r0 = val
	bne	1f		/ if (val == 0)
	inc	r0		/   r0 = 1
1:
	mov	r0,SC_R0(r1)	/ env->sc_r0 = r0 (`return' val)
	mov	2(sp),-(sp)	/ push env
	jsr	pc,_sigreturn	/ perform sigreturn(env)
	jsr	pc,__ljerr	/ if sigreturn returns, it's an error
	iot			/ and die if longjmperror returns
