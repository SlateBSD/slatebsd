/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)quota_sys.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * MELBOURNE QUOTAS
 *
 * System calls.
 */
#include "param.h"
#include "user.h"
#include "errno.h"

/*
 * The sys call that tells the system about a quota file.
 */
setquota()
{
	struct a {
		char	*fblk;
		char	*fname;
	};

	u.u_error = EINVAL;
}

/*
 * Sys call to allow users to find out
 * their current position wrt quota's
 * and to allow super users to alter it.
 */
qquota()
{
	struct a {
		int	cmd;
		int	uid;
		int	arg;
		caddr_t	addr;
	};

	u.u_error = EINVAL;
}
