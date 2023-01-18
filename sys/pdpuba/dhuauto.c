/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)dhuauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

dhuprobe(addr)
	u_int *addr;
{
	extern int errno;

	errno = 0;
	grab(addr);
	return(errno ? ACP_NXDEV : ACP_EXISTS);
}
