/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)rl.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 *  RL01/RL02 disk driver
 */

#include "rl.h"
#if NRL > 0

#include "param.h"
#include "buf.h"
#include "user.h"
#include "systm.h"
#include "conf.h"
#include "dk.h"
#include "map.h"
#include "uba.h"
#include "rlreg.h"

#define	RL01_NBLKS	10240	/* Number of UNIX blocks for an RL01 drive */
#define	RL02_NBLKS	20480	/* Number of UNIX blocks for an RL02 drive */
#define	RL_CYLSZ	10240	/* bytes per cylinder */
#define	RL_SECSZ	256	/* bytes per sector */

#define	rlwait(r)	while (((r)->rlcs & RL_CRDY) == 0)

struct	rldevice *RLADDR;

struct	buf	rrlbuf[NRL];	/* Raw header for each drive */
struct	buf	rlutab[NRL];	/* Seek structure for each device */
struct	buf	rltab;

struct	rl_softc {
	short	cn[4];		/* location of heads for each drive */
	short	type[4];	/* parameter dependent on drive type (RL01/2) */
	short	dn;		/* drive number */
	short	com;		/* read or write command word */
	short	chn;		/* cylinder and head number */
	u_short	bleft;		/* bytes left to be transferred */
	u_short	bpart;		/* number of bytes transferred */
	short	sn;		/* sector number */
	union	{
		short	w[2];
		long	l;
	} rl_un;		/* address of memory for transfer */

} rl = {-1,-1,-1,-1, -1,-1,-1,-1};	/* initialize cn[] and type[] */

#ifdef UCB_METER
static	int		rl_dkn = -1;	/* number for iostat */
#endif

rlroot()
{
	rlattach((struct rldevice *)0174400, 0);
}

rlattach(addr, unit)
	struct rldevice *addr;
{
#ifdef UCB_METER
	if (rl_dkn < 0) {
		dk_alloc(&rl_dkn, NRL+1, "rl", 40L * 40L * 128L);
		if (rl_dkn >= 0)
			dk_wps[rl_dkn+NRL] = 0L;
	}
#endif

	if (unit != 0)
		return (0);
	if ((addr != (struct rldevice *)NULL) && (fioword(addr) != -1)) {
		RLADDR = addr;
		return (1);
	}
	RLADDR = (struct rldevice *)NULL;
	return (0);
}

rlopen(dev)
	dev_t dev;
{
	if (minor(dev) >= NRL || !RLADDR)
		return (ENXIO);
	return (0);
}

rlstrategy(bp)
	register struct	buf *bp;
{
	register struct rldevice *rp;
	register int drive;
	int nblocks, s, ctr;

	drive = minor(bp->b_dev);
	if (drive >= NRL || !(rp = RLADDR)) {
		bp->b_error = ENXIO;
		goto bad;
	}

	/*
	 * We must determine what type of drive we are talking to in order 
	 * to determine how many blocks are on the device.  The rl.type[]
	 * array has been initialized with -1's so that we may test first
	 * contact with a particular drive and do this determination only once.
	 *
	 * For some unknown reason the RL02 (seems to be
	 * only drive 1) does not return a valid drive status
	 * the first time that a GET STATUS request is issued
	 * for the drive, in fact it can take up to three or more
	 * GET STATUS requests to obtain the correct status.
	 * In order to overcome this "HACK" the driver has been
	 * modified to issue a GET STATUS request, validate the
	 * drive status returned, and then use it to determine the
	 * drive type. If a valid status is not returned after eight
	 * attempts, then an error message is printed.
	 */
	if (rl.type[drive] < 0) {
		ctr = 0;
		do { /* get status and reset when first touching this drive */
			rp->rlda = RLDA_RESET | RLDA_GS;
			rp->rlcs = (drive << 8) | RL_GETSTATUS;	/* set up csr */
			rlwait(rp);
		} while (((rp->rlmp & 0177477) != 035) && (++ctr < 16));
		if (ctr >= 16) {
			printf("rl%d: no status\n", drive);
			printf("cs=%b da=%b\n", rp->rlcs, RL_BITS,
			    rp->rlda, RLDA_BITS);
			rl.type[drive] = RL02_NBLKS;	/* assume RL02 */
		} else if (rp->rlmp & RLMP_DTYP) {
			rl.type[drive] = RL02_NBLKS;	/* drive is RL02 */
		} else
			rl.type[drive] = RL01_NBLKS;	/* drive RL01 */
	}
	/* determine nblocks based upon which drive this is */
	nblocks = rl.type[drive];
	if(bp->b_blkno >= nblocks) {
		if((bp->b_blkno == nblocks) && (bp->b_flags & B_READ))
			bp->b_resid = bp->b_bcount;
		else {
			bp->b_error = ENXIO;
bad:
			bp->b_flags |= B_ERROR;
		}
		iodone(bp);
		return;
	}
#ifdef UNIBUS_MAP
	mapalloc(bp);
#endif

	bp->av_forw = NULL;
	bp->b_cylin = (int)(bp->b_blkno/20l);
	s = spl5();
	disksort(&rlutab[drive], bp);	/* Put the request on drive Q */
	if(rltab.b_active == NULL)
		rlstart();
	splx(s);
}

rlstart()
{
	register struct rl_softc *rlp = &rl;
	register struct buf *bp, *dp;
	int unit;

	if((bp = rltab.b_actf) == NULL) {
		for(unit = 0;unit < NRL;unit++) {	/* Start seeks */
			dp = &rlutab[unit];
			if(dp->b_actf == NULL)
				continue;
			rlseek((int)(dp->b_actf->b_blkno/20l),unit);
		}

		rlgss();	/* Put shortest seek on Q */
		if((bp = rltab.b_actf) == NULL)	/* No more work */
			return;
	}
	rltab.b_active++;
	rlp->dn = minor(bp->b_dev);
	rlp->chn = bp->b_blkno / 20;
	rlp->sn = (bp->b_blkno % 20) << 1;
	rlp->bleft = bp->b_bcount;
	rlp->rl_un.w[0] = bp->b_xmem & 077;
	rlp->rl_un.w[1] = (int) bp->b_un.b_addr;
	rlp->com = (rlp->dn << 8) | RL_IE;
	if (bp->b_flags & B_READ)
		rlp->com |= RL_RCOM;
	else
		rlp->com |= RL_WCOM;
	rlio();
}

rlintr()
{
	register struct buf *bp;
	register struct rldevice *rladdr = RLADDR;
	register status;

	if (rltab.b_active == NULL)
		return;
	bp = rltab.b_actf;
#ifdef UCB_METER
	if (rl_dkn >= 0)
		dk_busy &= ~((1 << (rl_dkn + rl.dn)) | (1 << (rl_dkn + NRL)));
#endif
	if (rladdr->rlcs & RL_CERR) {
		if (rladdr->rlcs & RL_HARDERR && rltab.b_errcnt > 2) {
			harderr(bp, "rl");
			printf("cs=%b da=%b\n", rladdr->rlcs, RL_BITS,
				rladdr->rlda, RLDA_BITS);
		}
		if (rladdr->rlcs & RL_DRE) {
			rladdr->rlda = RLDA_GS;
			rladdr->rlcs = (rl.dn <<  8) | RL_GETSTATUS;
			rlwait(rladdr);
			status = rladdr->rlmp;
			if(rltab.b_errcnt > 2) {
				harderr(bp, "rl");
				printf("mp=%b da=%b\n", status, RLMP_BITS,
					rladdr->rlda, RLDA_BITS);
			}
			rladdr->rlda = RLDA_RESET | RLDA_GS;
			rladdr->rlcs = (rl.dn << 8) | RL_GETSTATUS;
			rlwait(rladdr);
			if(status & RLMP_VCHK) {
				rlstart();
				return;
			}
		}
		if (++rltab.b_errcnt <= 10) {
			rl.cn[rl.dn] = -1;
			rlstart();
			return;
		}
		else {
			bp->b_flags |= B_ERROR;
			rl.bpart = rl.bleft;
		}
	}

	if ((rl.bleft -= rl.bpart) > 0) {
		rl.rl_un.l += rl.bpart;
		rl.sn=0;
		rl.chn++;
		rlseek(rl.chn,rl.dn);	/* Seek to new position */
		rlio();
		return;
	}
	bp->b_resid = 0;
	rltab.b_active = NULL;
	rltab.b_errcnt = 0;
	rltab.b_actf = bp->av_forw;
#ifdef notdef
	if((bp != NULL)&&(rlutab[rl.dn].b_actf != NULL))
		rlseek((int)(rlutab[rl.dn].b_actf->b_blkno/20l),rl.dn);
#endif
	iodone(bp);
	rlstart();
}

rlio()
{
	register struct rldevice *rladdr = RLADDR;

	if (rl.bleft < (rl.bpart = RL_CYLSZ - (rl.sn * RL_SECSZ)))
		rl.bpart = rl.bleft;
	rlwait(rladdr);
	rladdr->rlda = (rl.chn << 6) | rl.sn;
	rladdr->rlba = (caddr_t)rl.rl_un.w[1];
	rladdr->rlmp = -(rl.bpart >> 1);
#ifdef Q22
	rladdr->rlbae = rl.rl_un.w[0];
#endif
	rladdr->rlcs = rl.com | (rl.rl_un.w[0] & 03) << 4;
#ifdef UCB_METER
	if (rl_dkn >= 0) {
		int dkn = rl_dkn + NRL;

		dk_busy |= 1<<dkn;
		dk_xfer[dkn]++;
		dk_wds[dkn] += rl.bpart>>6;
	}
#endif
}

rlread(dev)
	register dev_t dev;
{
	return (physio(rlstrategy, &rrlbuf[minor(dev)], dev, B_READ, WORD));
}

rlwrite(dev)
	register dev_t dev;
{
	return (physio(rlstrategy, &rrlbuf[minor(dev)], dev, B_WRITE, WORD));
}

/*
 * Start a seek on an rl drive
 * Greg Travis, April 1982 - Adapted to 2.8/2.9 BSD Oct 1982/May 1984
 */
static
rlseek(cyl, dev)
	register int cyl;
	register int dev;
{
	struct rldevice *rp;
	register int dif;

	rp = RLADDR;
	if(rl.cn[dev] < 0)	/* Find the frigging heads */
		rlfh(dev);
	dif = (rl.cn[dev] >> 1) - (cyl >> 1);
	if(dif || ((rl.cn[dev] & 01) != (cyl & 01))) {
		if(dif < 0)
			rp->rlda = (-dif << 7) | RLDA_SEEKHI | ((cyl & 01) << 4);
		else
			rp->rlda = (dif << 7) | RLDA_SEEKLO | ((cyl & 01) << 4);
		rp->rlcs = (dev << 8) | RL_SEEK;
		rl.cn[dev] = cyl;
#ifdef UCB_METER
		if (rl_dkn >= 0) {
			int dkn = rl_dkn + dev;

			dk_busy |= 1<<dkn;	/* Mark unit busy */
			dk_seek[dkn]++;		/* Number of seeks */
		}
#endif
		rlwait(rp);	/* Wait for command */
	}
}

/* Find the heads for the given drive */
static
rlfh(dev)
	register int dev;
{
	register struct rldevice *rp;

	rp = RLADDR;
	rp->rlcs = (dev << 8) | RL_RHDR;
	rlwait(rp);
	rl.cn[dev] = ((unsigned)rp->rlmp & 0177700) >> 6;
}

/*
 * Find the shortest seek for the current drive and put
 * it on the activity queue
 */
static
rlgss()
{
	register int unit, dcn;
	register struct buf *dp;

	rltab.b_actf = NULL;	/* We fill this queue with up to 4 reqs */
	for(unit = 0;unit < NRL;unit++) {
		dp = rlutab[unit].b_actf;
		if(dp == NULL)
			continue;
		rlutab[unit].b_actf = dp->av_forw;	/* Out */
		dp->av_forw = dp->av_back = NULL;
		dcn = (dp->b_blkno/20) >> 1;
		if(rl.cn[unit] < 0)
			rlfh(unit);
		if(dcn < rl.cn[unit])
			dp->b_cylin = (rl.cn[unit] >> 1) - dcn;
		else
			dp->b_cylin = dcn - (rl.cn[unit] >> 1);
		disksort(&rltab, dp);	/* Put the request on the current q */
	}
}

#ifdef RL_DUMP
/*
 * Dump routine for RL01/02
 * Dumps from dumplo to end of memory/end of disk section for minor(dev).
 * It uses the UNIBUS map to dump all of memory if there is a UNIBUS map.
 * This routine is stupid (because the rl is stupid) and assumes that
 * dumplo begins on a track boundary!
 */

#define DBSIZE	10	/* Same number */

rldump(dev)
	dev_t dev;
{
	register struct rldevice *rladdr = RLADDR;
	daddr_t bn, dumpsize;
	long paddr;
	register int count;
	u_int com;
	int ccn, cn, tn, sn, unit, dif, ctr;
#ifdef UNIBUS_MAP
	register struct ubmap *ubp;
#endif

	unit = minor(dev);
	ctr = 0;
	do {			/* Determine drive type */
		rladdr->rlda = RLDA_RESET | RLDA_GS;
		rladdr->rlcs = (dev << 8) | RL_GETSTATUS;
		rlwait(rladdr);
	} while(((rladdr->rlmp & 0177477) != 035) && (++ctr < 16));
	if(ctr >= 16) {
		printf("rl%d: no status\n",dev);
		return(EIO);
	}
	if(rladdr->rlmp & RLMP_DTYP)
		dumpsize = RL02_NBLKS;
	else
		dumpsize = RL01_NBLKS;
	if((dumplo < 0) || (dumplo >= dumpsize))
		return(EINVAL);
	dumpsize -= dumplo;

	rladdr->rlcs = (dev << 8) | RL_RHDR;	/* Find the heads */
	rlwait(rladdr);
	ccn = ((unsigned)rladdr->rlmp&0177700) >> 6;

#ifdef UNIBUS_MAP
	ubp = &UBMAP[0];
#endif
	for(paddr = 0L;dumpsize > 0;dumpsize -= count) {
		count = dumpsize > DBSIZE ? DBSIZE : dumpsize;
		bn = dumplo + (paddr >> PGSHIFT);
		cn = bn / 20;
		sn = (unsigned)(bn % 20) << 1;
		dif = (ccn >> 1) - (cn >> 1);
		if(dif || ((ccn & 01) != (cn & 01))) {
			if(dif < 0)
				rladdr->rlda = (-dif << 7) | RLDA_SEEKHI |
					((cn & 01) << 4);
			else
				rladdr->rlda = (dif << 7) | RLDA_SEEKLO |
					((cn & 01) << 4);
			rladdr->rlcs = (dev << 8) | RL_SEEK;
			ccn = cn;
			rlwait(rladdr);
		}
		rladdr->rlda = (cn << 6) | sn;
		rladdr->rlmp = -(count << (PGSHIFT-1));
		com = (dev << 8) | RL_WCOM;
#ifdef UNIBUS_MAP
		/* If there is a map - use it */
		if(ubmap) {
			ubp->ub_lo = loint(paddr);
			ubp->ub_hi = hiint(paddr);
			rladdr->rlba = 0;
		} else {
#endif
			rladdr->rlba = loint(paddr);
#ifdef Q22
			rladdr->rlbae = hiint(paddr);
#endif
			com |= (hiint(paddr) & 03) << 4;
#ifdef UNIBUS_MAP
		}
#endif
		rladdr->rlcs = com;
		rlwait(rladdr);
		if(rladdr->rlcs & RL_CERR) {
			if(rladdr->rlcs & RL_NXM)
				return(0);	/* End of memory */
			printf("rl%d: dmp err, cs=%b da=%b mp=%b\n",
				dev,rladdr->rlcs,RL_BITS,rladdr->rlda,
				RLDA_BITS, rladdr->rlmp, RLMP_BITS);
			return(EIO);
		}
		paddr += (DBSIZE << PGSHIFT);
	}
	return(0);	/* Filled the disk */
}
#endif RL_DUMP
#endif
