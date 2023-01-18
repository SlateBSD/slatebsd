/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)rxauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

#include "rxreg.h"

/*
 * rxprobe - check for rx
 */
rxprobe(addr)
	struct rxdevice *addr;
{
	stuff(RX_INIT | RX_IE, (&(addr)->rxcs));
	DELAY(1000);
	stuff(0, (&(addr)->rxcs));
	return(ACP_IFINTR);
}
