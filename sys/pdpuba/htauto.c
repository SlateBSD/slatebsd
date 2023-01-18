/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)htauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

#include "htreg.h"

htprobe(addr)
	struct htdevice *addr;
{
	stuff(HT_SENSE | HT_IE | HT_GO, (&(addr->htcs1)));
	DELAY(10);
	stuff(0, (&(addr->htcs1)));
	/*
	 * clear error condition, or driver will report an error first
	 * time you open it after the boot.
	 */
	stuff(HTCS2_CLR, (&(addr->htcs2)));
	return(ACP_IFINTR);
}
