/*
 * C runtime startoff
 * with floating point interpreter
 */
.globl	fptrap, _exit, _main, _environ

#include "SYS.h"
/ can't include <sys/signal.h> because of C declarations ...
#define	SIGILL	4

start:
	mov	$fptrap,-(sp)
	mov	$SIGILL,-(sp)
	tst	-(sp)		/ simulate return address stack spacing
	SYS(signal)
	add	$6,sp
	setd
	mov	2(sp),r0
	clr	-2(r0)
	mov	sp,r0
	sub	$4,sp
	mov	4(sp),(sp)
	tst	(r0)+
	mov	r0,2(sp)
1:
	tst	(r0)+
	bne	1b
	cmp	r0,*2(sp)
	blo	1f
	tst	-(r0)
1:
	mov	r0,4(sp)
	mov	r0,_environ
	jsr	pc,_main
	cmp	(sp)+,(sp)+
	mov	r0,(sp)
	jsr	pc,*$_exit
	SYS(exit)
.bss
_environ:
	.=.+2
.data
	.=.+2
