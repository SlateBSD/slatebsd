/*
 * XXX - Is this routine used??!
 *
 * fakefp -- fake floating point simulator
 */
#include "SYS.h"
/ can't include <sys/signal.h> because of C declarations ...
#define	SIGILL	4
#define	SIG_DFL	0

rti	= 2

.globl	fptrap		/ Can't use PROFCODE because it would change
fptrap:			/   r0 & r1 in libc_p.a
	sub	$2,(sp)
	mov	r0,-(sp)
	mov	$SIG_DFL,-(sp)
	mov	$SIGILL,-(sp)
	SYS(signal)
	cmp	(sp)+,(sp)+
	mov	(sp)+,r0
	rti
