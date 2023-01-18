/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)cauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "cons.h"

cnprobe(addr)
struct dldevice *addr;
{
	stuff(grab(&(addr->dlxcsr)) | DLXCSR_TIE, &(addr->dlxcsr));
	DELAY(35000);
	DELAY(35000);
	/*
	 *  Leave TIE enabled; cons.c never turns it off
	 *  (and this could be the console).
	 */
	return(ACP_IFINTR);
}
