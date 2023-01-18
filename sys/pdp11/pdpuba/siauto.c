/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)siauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

#include "sireg.h"

siprobe(addr)
	struct sidevice *addr;
{
	stuff(SI_IE | SI_READ, &(addr->sicnr));
	stuff(SI_IE | SI_READ | SI_DONE, &(addr->sicnr));
	DELAY(10);
	stuff(0, &(addr->sicnr));
	return(0);
}
