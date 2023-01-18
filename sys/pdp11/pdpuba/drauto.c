/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)drauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

#include "drreg.h"

drprobe(addr)
	struct drdevice *addr;
{
	stuff(DR_MANT, &(addr->csr));	/* toggle maintence bit */
	stuff(0, &(addr->csr));		/* to reset dr11 */
	return(ACP_EXISTS);		/* can't make it interrupt */
}
