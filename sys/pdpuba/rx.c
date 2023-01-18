/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)rx.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * RX02 floppy disk device driver
 *
 * This driver was written by Bill Shannon and distributed on the
 * DEC v7m UNIX tape.  It has been modified for 2BSD and has been
 * included with the permission of the DEC UNIX Engineering Group.
 *
 * Modified to actually work with 2.9BSD by Gregory Travis, Indiana Univ.
 *
 *	Layout of logical devices:
 *
 *	name	min dev		unit	density
 *	----	-------		----	-------
 *	rx0a	   0		  0	single
 *	rx1a	   1		  1	single
 *	rx0b	   2		  0	double
 *	rx1b	   3		  1	double
 *
 *	ioctl function call may be used to format a disk.
 */

#include "rx.h"
#if NRX > 0
#include "param.h"
#include "buf.h"
#include "conf.h"
#include "ioctl.h"
#include "tty.h"
#include "rxreg.h"
#include "errno.h"

struct	rxdevice *RXADDR;

/*
 *	the following defines use some fundamental
 *	constants of the RX02.
 */
#define	NSPB	((minor(bp->b_dev)&2) ? 2 : 4)		/* sectors per block */
#define	NRXBLKS	((minor(bp->b_dev)&2) ? 1001 : 500)	/* blocks on device */
#define	NBPS	((minor(bp->b_dev)&2) ? 256 : 128)	/* bytes per sector */
#define	DENSITY	(minor(bp->b_dev)&2)	/* Density: 0 = single, 2 = double */
#define	UNIT	(minor(bp->b_dev)&1)	/* Unit Number: 0 = left, 1 = right */

#define	rxwait()	while (((RXADDR->rxcs) & RX_XREQ) == 0)
#define	seccnt(bp)	((int)((bp)->b_seccnt))

struct	buf	rxtab;
struct	buf	rrxbuf;
struct	buf	crxbuf;		/* buffer header for control functions */

/*
 *	states of driver, kept in b_state
 */
#define	SREAD	1	/* read started */
#define	SEMPTY	2	/* empty started */
#define	SFILL	3	/* fill started */
#define	SWRITE	4	/* write started */
#define	SINIT	5	/* init started */
#define	SFORMAT	6	/* format started */

rxattach(addr, unit)
	struct rxdevice *addr;
	u_int unit;
{
	if (unit != 0)
		return (0);
	RXADDR = addr;
	return (1);
}

/*ARGSUSED*/
rxopen(dev, flag)
	dev_t dev;
{
	if (minor(dev) >= 4 || !RXADDR)
		return (ENXIO);
	return (0);
}

rxstrategy(bp)
	register struct buf *bp;
{
	register int s;

	if (minor(bp->b_dev) >= 4 || !RXADDR)
		goto bad;
#ifdef UNIBUS_MAP
	mapalloc(bp);
#endif
	if (bp->b_blkno >= NRXBLKS) {
		if (bp->b_flags&B_READ)
			bp->b_resid = bp->b_bcount;
		else {
bad:			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = (struct buf *) NULL;

	/*
	 * seccnt is actually the number of floppy sectors transferred,
	 * incremented by one after each successful transfer of a sector.
	 */
	bp->b_seccnt = 0;

	/*
	 * We'll modify b_resid as each piece of the transfer
	 * successfully completes.  It will also tell us when
	 * the transfer is complete.
	 */
	bp->b_resid = bp->b_bcount;
	s = splbio();
	if (rxtab.b_actf == NULL)
		rxtab.b_actf = bp;
	else
		rxtab.b_actl->av_forw = bp;
	rxtab.b_actl = bp;
	if (rxtab.b_state == NULL)
		rxstart();
	splx(s);
}

rxstart()
{
	register struct buf *bp;
	int sector, track;
	char *addr, xmem;

	if ((bp = rxtab.b_actf) == NULL) {
		rxtab.b_state = NULL;
		return;
	}

	if (bp == &crxbuf) {		/* is it a control request ? */
		rxtab.b_state = SFORMAT;
		RXADDR->rxcs = RX_SMD | RX_GO | RX_IE | (UNIT << 4) | (DENSITY << 7);
		rxwait();
		RXADDR->rxdb = 'I';
	} else
	if (bp->b_flags & B_READ) {
		rxtab.b_state = SREAD;
		rxfactr((int)bp->b_blkno * NSPB + seccnt(bp), &sector, &track);
		RXADDR->rxcs = RX_RSECT | RX_GO | RX_IE | (UNIT << 4) | (DENSITY << 7);
		rxwait();
		RXADDR->rxsa = sector;
		rxwait();
		RXADDR->rxta = track;
	} else {
		rxtab.b_state = SFILL;
		rxaddr(bp, &addr, &xmem);
		RXADDR->rxcs = RX_FILL | RX_GO | RX_IE | ((u_int)xmem << 12) | (DENSITY << 7);
		rxwait();
		RXADDR->rxwc = (bp->b_resid >= NBPS ? NBPS : bp->b_resid) >> 1;
		rxwait();
		RXADDR->rxba = (short)addr;
	}
}

rxintr()
{
	register struct buf *bp;
	int sector, track;
	char addr, xmem;

	if (rxtab.b_state == SINIT) {
		rxstart();
		return;
	}

	if ((bp = rxtab.b_actf) == NULL)
		return;

	if (RXADDR->rxcs < 0) {
		if (rxtab.b_errcnt++ > 10 || rxtab.b_state == SFORMAT) {
			bp->b_flags |= B_ERROR;
			harderr(bp, "rx");
			printf("cs=%b er=%b\n", RXADDR->rxcs, RX_BITS,
				RXADDR->rxes, RXES_BITS);
			rxtab.b_errcnt = 0;
			rxtab.b_actf = bp->av_forw;
			iodone(bp);
		}
		RXADDR->rxcs = RX_INIT;
		RXADDR->rxcs = RX_IE;
		rxtab.b_state = SINIT;
		return;
	}
	switch (rxtab.b_state) {

	case SREAD:			/* read done, start empty */
		rxtab.b_state = SEMPTY;
		rxaddr(bp, &addr, &xmem);
		RXADDR->rxcs = RX_EMPTY | RX_GO | RX_IE | ((u_int)xmem << 12) | (DENSITY << 7);
		rxwait();
		RXADDR->rxwc = (bp->b_resid >= NBPS? NBPS : bp->b_resid) >> 1;
		rxwait();
		RXADDR->rxba = (short)addr;
		return;

	case SFILL:			/* fill done, start write */
		rxtab.b_state = SWRITE;
		rxfactr((int)bp->b_blkno * NSPB + seccnt(bp), &sector, &track);
		RXADDR->rxcs = RX_WSECT | RX_GO | RX_IE | (UNIT << 4) | (DENSITY << 7);
		rxwait();
		RXADDR->rxsa = sector;
		rxwait();
		RXADDR->rxta = track;
		return;

	case SWRITE:			/* write done, start next fill */
	case SEMPTY:			/* empty done, start next read */
		/*
		 * increment amount remaining to be transferred.
		 * if it becomes positive, last transfer was a
		 * partial sector and we're done, so set remaining
		 * to zero.
		 */
		if (bp->b_resid <= NBPS) {
done:
			bp->b_resid = 0;
			rxtab.b_errcnt = 0;
			rxtab.b_actf = bp->av_forw;
			iodone(bp);
			break;
		}

		bp->b_resid -= NBPS;
		bp->b_seccnt++;
		break;

	case SFORMAT:			/* format done (whew!!!) */
		goto done;		/* driver's getting too big... */
	}

	/* end up here from states SWRITE and SEMPTY */
	rxstart();
}

/*
 *	rxfactr -- calculates the physical sector and physical
 *	track on the disk for a given logical sector.
 *	call:
 *		rxfactr(logical_sector,&p_sector,&p_track);
 *	the logical sector number (0 - 2001) is converted
 *	to a physical sector number (1 - 26) and a physical
 *	track number (0 - 76).
 *	the logical sectors specify physical sectors that
 *	are interleaved with a factor of 2. thus the sectors
 *	are read in the following order for increasing
 *	logical sector numbers (1,3, ... 23,25,2,4, ... 24,26)
 *	There is also a 6 sector slew between tracks.
 *	Logical sectors start at track 1, sector 1; go to
 *	track 76 and then to track 0.  Thus, for example, unix block number
 *	498 starts at track 0, sector 25 and runs thru track 0, sector 2
 *	(or 6 depending on density).
 */
static
rxfactr(sectr, psectr, ptrck)
	register int sectr;
	int *psectr, *ptrck;
{
	register int p1, p2;

	p1 = sectr / 26;
	p2 = sectr % 26;
	/* 2 to 1 interleave */
	p2 = (2 * p2 + (p2 >= 13 ?  1 : 0)) % 26;
	/* 6 sector per track slew */
	*psectr = 1 + (p2 + 6 * p1) % 26;
	if (++p1 >= 77)
		p1 = 0;
	*ptrck = p1;
}


/*
 *	rxaddr -- compute core address where next sector
 *	goes to / comes from based on bp->b_un.b_addr, bp->b_xmem,
 *	and seccnt(bp).
 */
static
rxaddr(bp, addr, xmem)
	register struct buf *bp;
	register char **addr, *xmem;
{
	*addr = bp->b_un.b_addr + seccnt(bp) * NBPS;
	*xmem = bp->b_xmem;
	if (*addr < bp->b_un.b_addr)		/* overflow, bump xmem */
		(*xmem)++;
}

rxread(dev)
	dev_t dev;
{
	return (physio(rxstrategy, &rrxbuf, dev, B_READ, WORD));
}


rxwrite(dev)
	dev_t dev;
{
	return (physio(rxstrategy, &rrxbuf, dev, B_WRITE, WORD));
}

/*
 *	rxioctl -- format RX02 disk, single or double density.
 *	density determined by device opened.
 */
/*ARGSUSED*/
rxioctl(dev, cmd, addr, flag)
	dev_t dev;
	u_int cmd;
{
	register int s;
	register struct buf *bp;

	if (cmd != RXIOC_FORMAT)
		return (ENXIO);
	bp = &crxbuf;
	while (bp->b_flags & B_BUSY) {
		s = splbio();
		bp->b_flags |= B_WANTED;
		sleep(bp, PRIBIO);
	}
	splx(s);
	bp->b_flags = B_BUSY;
	bp->b_dev = dev;
	bp->b_error = 0;
	rxstrategy(bp);
	iowait(bp);
	bp->b_flags = 0;
	return (0);
}
#endif
