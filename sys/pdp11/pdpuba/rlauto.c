/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)rlauto.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

#include "rlreg.h"

rlprobe(addr)
	struct rldevice *addr;
{
	stuff(RL_NOP | RL_IE, (&(addr->rlcs)));
	DELAY(10);
	stuff(RL_CRDY, (&(addr->rlcs)));
	return(ACP_IFINTR);
}
