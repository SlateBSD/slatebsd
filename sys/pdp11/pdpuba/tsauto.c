/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)tsauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

#include "tmreg.h"

tsprobe(addr)
	struct tsdevice *addr;
{
	extern int errno;

	errno = 0;		/* assume exists if no tm-11 there */
	grab(&(((struct tmdevice *)addr)->tmba));
	return(errno ? ACP_EXISTS : ACP_NXDEV);
}
