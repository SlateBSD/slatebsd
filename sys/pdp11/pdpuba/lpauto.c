/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)lpauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

/*
 * LP_IE and lpdevice copied from lp.c!!!
 */
#define	LP_IE		0100		/* interrupt enable */

struct lpdevice {
	short	lpcs;
	short	lpdb;
};

lpprobe(addr)
	struct lpdevice	*addr;
{
	stuff(grab(&(addr->lpcs)) | LP_IE, &(addr->lpcs));
	DELAY(10);
	stuff(0, &(addr->lpcs));
	return(ACP_IFINTR);
}
