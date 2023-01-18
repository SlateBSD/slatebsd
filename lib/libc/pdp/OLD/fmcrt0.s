/*
 * C runtime startoff including monitoring
 */
.globl	_monitor, _sbrk, _main, _exit, _environ, _etext, __cleanup
.globl	countbase, fptrap

#include "SYS.h"
/ can't include <sys/signal.h> because of C declarations ...
#define	SIGILL	4

cbufs	= 300.

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

	mov	$_etext,r1
	sub	$eprol,r1
	add	$7,r1
	ash	$-3,r1
	bic	$!17777,r1
	mov	$cbufs,-(sp)
	add	$3*[cbufs+1],r1
	mov	r1,-(sp)
	asl	r1
	mov	r1,-(sp)
	jsr	pc,_sbrk
	tst	(sp)+
	cmp	r0,$-1
	beq	9f
	mov	r0,-(sp)
	add	$6,r0
	mov	r0,countbase
	mov	$_etext,-(sp)
	mov	$eprol,-(sp)
	jsr	pc,_monitor
	add	$10.,sp
	jsr	pc,_main
	cmp	(sp)+,(sp)+
	mov	r0,(sp)
	jsr	pc,_exit

9:
	mov	$9f-8f,-(sp)
	mov	$8f,-(sp)
	mov	$2,-(sp)
	tst	-(sp)		/ simulate return address stack spacing
	SYS(write)
	add	$8,sp

.data
8:	<No space for monitor buffer\n>
9:	.even
.text

_exit:
	jsr	pc,__cleanup
	clr	-(sp)
	jsr	pc,_monitor
	tst	(sp)+
	SYS(exit)
eprol:
.bss
_environ:
	.=.+2
countbase:
	.=.+2
.data
	.=.+2
