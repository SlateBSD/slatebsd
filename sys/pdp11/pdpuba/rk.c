/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)rk.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * RK05 device drive
 */

#include "rk.h"
#if	NRK > 0
#include "param.h"
#include "systm.h"
#include "buf.h"
#include "conf.h"
#include "user.h"
#include "dk.h"
#include "rkreg.h"

#define	NRKBLK	4872	/* Number of blocks per drive */

struct	rkdevice *RKADDR;

struct	buf	rktab;
struct	buf	rrkbuf[NRK];

#define	rkunit(dev)	((minor(dev) >> 3) & 07)

#ifdef UCB_METER
static	int		rk_dkn = -1;	/* number for iostat */
#endif

rkattach(addr, unit)
struct rkdevice *addr;
{
#ifdef UCB_METER
	if (rk_dkn < 0)
		dk_alloc(&rk_dkn, 1, "rk", 25L * 12L * 256L);
#endif

	if (unit != 0)
		return(0);
	RKADDR = addr;
	return(1);
}

rkopen(dev)
	dev_t dev;
{
	register int unit = rkunit(dev);

	if (unit >= NRK || !RKADDR)
		return (ENXIO);
	return (0);
}

rkstrategy(bp)
register struct buf *bp;
{
	register int s;
	register int unit;

	unit = rkunit(bp->b_dev);
	if (unit >= NRK || !RKADDR) {
		bp->b_error = ENXIO;
		goto bad;
	}
	if (bp->b_blkno >= NRKBLK) {
		bp->b_error = EINVAL;
bad:		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
#ifdef UNIBUS_MAP
	mapalloc(bp);
#endif
	bp->av_forw = (struct buf *)NULL;
	s = spl5();
	if(rktab.b_actf == NULL)
		rktab.b_actf = bp;
	else
		rktab.b_actl->av_forw = bp;
	rktab.b_actl = bp;
	if(rktab.b_active == NULL)
		rkstart();
	splx(s);
}

rkstart()
{
	register struct	rkdevice *rkaddr = RKADDR;
	register struct buf *bp;
	register com;
	daddr_t bn;
	int dn, cn, sn;

	if ((bp = rktab.b_actf) == NULL)
		return;
	rktab.b_active++;
	bn = bp->b_blkno;
	dn = minor(bp->b_dev);
	cn = bn / 12;
	sn = bn % 12;
	rkaddr->rkda = (dn << 13) | (cn << 4) | sn;
	rkaddr->rkba = bp->b_un.b_addr;
	rkaddr->rkwc = -(bp->b_bcount >> 1);
	com = ((bp->b_xmem & 3) << 4) | RKCS_IDE | RKCS_GO;
	if(bp->b_flags & B_READ)
		com |= RKCS_RCOM;
	else
		com |= RKCS_WCOM;
	rkaddr->rkcs = com;

#ifdef UCB_METER
	if (rk_dkn >= 0) {
		dk_busy |= 1<<rk_dkn;
		dk_seek[rk_dkn]++;
		dk_wds[rk_dkn] += bp->b_bcount>>6;
	}
#endif
}

rkintr()
{
	register struct rkdevice *rkaddr = RKADDR;
	register struct buf *bp;

	if (rktab.b_active == NULL)
		return;
#ifdef UCB_METER
	if (rk_dkn >= 0)
		dk_busy &= ~(1<<rk_dkn);
#endif
	bp = rktab.b_actf;
	rktab.b_active = NULL;
	if (rkaddr->rkcs & RKCS_ERR) {
		while ((rkaddr->rkcs & RKCS_RDY) == 0)
			;
		if (rkaddr->rker & RKER_WLO)
			/*
			 *	Give up on write locked devices
			 *	immediately.
			 */
			printf("rk%d: write locked\n", minor(bp->b_dev));
		else
			{
			harderr(bp, "rk");
			printf("er=%b ds=%b\n", rkaddr->rker, RKER_BITS,
				rkaddr->rkds, RK_BITS);
			rkaddr->rkcs = RKCS_RESET | RKCS_GO;
			while((rkaddr->rkcs & RKCS_RDY) == 0)
				;
			if (++rktab.b_errcnt <= 10) {
				rkstart();
				return;
			}
		}
		bp->b_flags |= B_ERROR;
	}
	rktab.b_errcnt = 0;
	rktab.b_actf = bp->av_forw;
	bp->b_resid = -(rkaddr->rkwc << 1);
	iodone(bp);
	rkstart();
}

rkread(dev)
	register dev_t dev;
{
	return (physio(rkstrategy, &rrkbuf[rkunit(dev)], dev, B_READ, WORD));
}

rkwrite(dev)
	register dev_t dev;
{
	return (physio(rkstrategy, &rrkbuf[rkunit(dev)], dev, B_WRITE, WORD));
}
#endif NRK
