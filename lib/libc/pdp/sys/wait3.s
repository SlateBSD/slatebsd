/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid: <@(#)wait3.s	2.5 (Berkeley) 1/29/87\0>
	.even
#endif SYSLIBC_SCCS

/*
 * XXX - this routine can't use SYSCALL!!!
 *
 * pid = wait3(status, flags, rusage);
 *	int		pid;
 *	union wait	*status;
 *	int		flags;
 *	struct rusage	*rusage;
 *
 * Pid == -1 if error.  Status indicates fate of process, if given, flags may
 * indicate process is not to hang or that untraced stopped children are to be
 * reported.  Rusage optionally returns detailed resource usage information.
 *
 * Wait and wait3 are the same system call with wait3 being distinguished via
 * entering the kernel with all condition codes set.  Wait is defined as
 * having no parameters in init_sysent.c, so the code for wait in kern_exit.c
 * grabs it's extra parameters from r0 and r1 when it detects PSL_ALLCC ...
 */
#include "SYS.h"

ENTRY(wait3)
	mov	4(sp),r0	/ r0 = flags
	mov	6(sp),r1	/ r1 = rusage
	sec|sev|sez|sen
	SYS(wait)
	bes	2f
	tst	2(sp)		/ Does the user want the terminated child's
	beq	1f		/   status?
	mov	r1,*2(sp)	/   sure - so give it to them
1:
	rts	pc
2:
	mov	r0,_errno
	mov	$-1,r0
	rts	pc
