/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)si.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 *	SI 9500 CDC 9766 Stand Alone disk driver
 */

#include <sys/param.h>

#include "../pdpuba/sireg.h"

#include <sys/inode.h>
#include "saio.h"

#define SIADDR ((struct sidevice *)0176700)

/* defines for 9766 */
#define NHEADS	19
#define NSECT	32

#define NSPC	NSECT*NHEADS
static int dualsi;

sistrategy(io, func)
register struct iob *io;
{
	register unit;
	register ii;
	daddr_t bn;
	int sn, cn, tn;

	if(((unit = io->i_unit) & 04) == 0)
		bn = io->i_bn;
	else
	{
		unit &= 03;
		bn = io->i_bn;
		bn -= io->i_boff;
		ii = unit + 1;
		unit = bn % ii;
		bn /= ii;
		bn += io->i_boff;
	}

	cn = bn / (NSPC);
	sn = bn % (NSPC);
	tn = sn / (NSECT);
	sn = sn % (NSECT);

	if(!dualsi)
	{
		if(SIADDR->siscr != 0)
		{
			dualsi++;
		}
		else
		{
			if((SIADDR->sierr & (SIERR_ERR | SIERR_CNT)) == (SIERR_ERR | SIERR_CNT))
			{
				dualsi++;
			}
		}
	}
	if(dualsi)
	{
		while(!(SIADDR->siscr & 0200))
		{
			SIADDR->sicnr = SI_RESET;
			SIADDR->siscr = 1;
		}
	}
	SIADDR->sipcr = cn;
	SIADDR->sihsr = (tn << 5) + sn;
	SIADDR->simar = io->i_ma;
	SIADDR->siwcr = io->i_cc >> 1;
	/*
	 * warning: unit is being used as a temporary.
	 */
	unit = ((segflag & 03) << 4) | SI_GO;
	if(func == READ)
		unit |= SI_READ;
	else if(func == WRITE)
		unit |= SI_WRITE;
	
	SIADDR->sicnr = unit;

	while((SIADDR->sicnr & SI_DONE) == 0)
		;
	
	if(SIADDR->sierr & SIERR_ERR)
	{
		printf("disk error cyl=%d head=%d sect=%d cnr=%o, err=%o\n",
			cn, tn, sn, SIADDR->sicnr, SIADDR->sierr);
		return(-1);
	}
	return(io->i_cc);
}
