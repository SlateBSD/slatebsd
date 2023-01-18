/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_rtp.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "user.h"
#include "proc.h"

rtp()
{
	struct a {
		int flag;
	};

#ifdef CGL_RTP
	if (!suser())
		return;
	if (((struct a *)u.u_ap)->flag) {
		if (rtpp) {
			u.u_error = EBUSY;	/* exclusive use error */
			return;
		}
		u.u_procp->p_flag |= SULOCK;	/* lock in core flag */
		rtpp = u.u_procp;
	}
	else {
		u.u_procp->p_flag &= ~SULOCK;
		rtpp = NULL;
	}
#else
	u.u_error = EINVAL;
#endif
}

#ifdef CGL_RTP
/*
 * Test status of the "real time process"; preempt the current process if
 * runnable.  Use caution when calling this routine, much of the kernel is
 * non-reentrant!
 */
runrtp()
{
	register struct proc	*p;

	if ((p = rtpp) == NULL || p == u.u_procp)
		return;
	if (p->p_stat == SRUN && (p->p_flag & SLOAD) != 0) {
		u.u_procp->p_pri = PRTP+1;
		setrq(u.u_procp);
		swtch();
	}
}
#endif
