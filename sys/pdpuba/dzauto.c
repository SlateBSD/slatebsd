/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)dzauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

#include "dzreg.h"

dzprobe(addr)
	struct dzdevice *addr;
{
	stuff(grab(&(addr->dzcsr)) | DZ_TIE | DZ_MSE, &(addr->dzcsr));
	stuff(1, &(addr->dztcr));
	DELAY(35000);
	DELAY(35000);
	stuff(DZ_CLR, &(addr->dzcsr));
	return(ACP_IFINTR);
}
