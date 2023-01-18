/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)rkauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

#include "rkreg.h"

rkprobe(addr)
	struct rkdevice *addr;
{
	stuff(RKCS_IDE | RKCS_DRESET | RKCS_GO, (&(addr->rkcs)));
	DELAY(10);
	stuff(0, (&(addr->rkcs)));
	return(ACP_IFINTR);
}
