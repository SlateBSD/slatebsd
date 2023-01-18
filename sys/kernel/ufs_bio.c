/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ufs_bio.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "buf.h"
#include "user.h"
#include "conf.h"
#include "fs.h"
#include "dk.h"
#include "systm.h"
#include "map.h"
#include "uba.h"
#include "trace.h"
#include "ram.h"

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
struct buf *
bread(dev, blkno)
	dev_t dev;
	daddr_t blkno;
{
	register struct buf *bp;

	bp = getblk(dev, blkno);
	if (bp->b_flags&B_DONE) {
		trace(TR_BREADHIT);
		return (bp);
	}
	bp->b_flags |= B_READ;
	bp->b_bcount = DEV_BSIZE;	/* XXX? KB */
	(*bdevsw[major(dev)].d_strategy)(bp);
	trace(TR_BREADMISS);
#ifdef UCB_RUSAGE
	u.u_ru.ru_inblock++;		/* pay for read */
#endif
	biowait(bp);
	return(bp);
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 */
struct buf *
breada(dev, blkno, rablkno)
	register dev_t dev;
	daddr_t blkno;
	daddr_t rablkno;
{
	register struct buf *bp, *rabp;

	bp = NULL;
	/*
	 * If the block isn't in core, then allocate
	 * a buffer and initiate i/o (getblk checks
	 * for a cache hit).
	 */
	if (!incore(dev, blkno)) {
		bp = getblk(dev, blkno);
		if ((bp->b_flags&B_DONE) == 0) {
			bp->b_flags |= B_READ;
			bp->b_bcount = DEV_BSIZE;	/* XXX? KB */
			(*bdevsw[major(dev)].d_strategy)(bp);
			trace(TR_BREADMISS);
#ifdef UCB_RUSAGE
			u.u_ru.ru_inblock++;		/* pay for read */
#endif
		}
		else
			trace(TR_BREADHIT);
	}

	/*
	 * If there's a read-ahead block, start i/o
	 * on it also (as above).
	 */
	if (rablkno && !incore(dev, rablkno)) {
		rabp = getblk(dev, rablkno);
		if (rabp->b_flags & B_DONE) {
			brelse(rabp);
			trace(TR_BREADHITRA);
		} else {
			rabp->b_flags |= B_READ|B_ASYNC;
			rabp->b_bcount = DEV_BSIZE;	/* XXX? KB */
			(*bdevsw[major(dev)].d_strategy)(rabp);
			trace(TR_BREADMISSRA);
#ifdef UCB_RUSAGE
			u.u_ru.ru_inblock++;		/* pay in advance */
#endif
		}
	}

	/*
	 * If block was in core, let bread get it.
	 * If block wasn't in core, then the read was started
	 * above, and just wait for it.
	 */
	if (bp == NULL)
		return (bread(dev, blkno));
	biowait(bp);
	return (bp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
	register struct buf *bp;
{
	register flag;

	flag = bp->b_flags;
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
#ifdef UCB_RUSAGE
	if ((flag&B_DELWRI) == 0)
		u.u_ru.ru_oublock++;		/* noone paid yet */
#endif
	trace(TR_BWRITE);
	bp->b_bcount = DEV_BSIZE;		/* XXX? KB */
	(*bdevsw[major(bp->b_dev)].d_strategy)(bp);

	/*
	 * If the write was synchronous, then await i/o completion.
	 * If the write was "delayed", then we put the buffer on
	 * the q of blocks awaiting i/o completion status.
	 */
	if ((flag&B_ASYNC) == 0) {
		biowait(bp);
		brelse(bp);
	} else if (flag & B_DELWRI)
		bp->b_flags |= B_AGE;
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp)
	register struct buf *bp;
{

#ifdef UCB_RUSAGE
	if ((bp->b_flags&B_DELWRI) == 0)
		u.u_ru.ru_oublock++;		/* noone paid yet */
#endif
	if (bdevsw[major(bp->b_dev)].d_flags & B_TAPE) {
		bawrite(bp);
	}
	else {
		bp->b_flags |= B_DELWRI | B_DONE;
		brelse(bp);
	}
}

/*
 * Release the buffer, with no I/O implied.
 */
brelse(bp)
	register struct buf *bp;
{
	register struct buf *flist;
	register s;

	trace(TR_BRELSE);
	/*
	 * If someone's waiting for the buffer, or
	 * is waiting for a buffer, wake 'em up.
	 */
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	if (bfreelist[0].b_flags&B_WANTED) {
		bfreelist[0].b_flags &= ~B_WANTED;
		wakeup((caddr_t)bfreelist);
	}
	if (bp->b_flags&B_ERROR)
		bp->b_dev = NODEV;	/* no assoc */

	/*
	 * Stick the buffer back on a free list.
	 */
	s = splbio();
	if (bp->b_flags & (B_ERROR|B_INVAL)) {
		/* block has no info ... put at front of most free list */
		flist = &bfreelist[BQ_AGE];
		binsheadfree(bp, flist);
	} else {
		if (bp->b_flags & B_AGE)
			flist = &bfreelist[BQ_AGE];
		else
			flist = &bfreelist[BQ_LRU];
		binstailfree(bp, flist);
	}
	bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC|B_AGE);
	splx(s);
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */
incore(dev, blkno)
	register dev_t dev;
	daddr_t blkno;
{
	register struct buf *bp;
	register struct buf *dp;

	dp = BUFHASH(blkno);
	blkno = fsbtodb(blkno);
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
		if (bp->b_blkno == blkno && bp->b_dev == dev &&
		    (bp->b_flags & B_INVAL) == 0)
			return (1);
	return (0);
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 *
 * We use splx here because this routine may be called
 * on the interrupt stack during a dump, and we don't
 * want to lower the ipl back to 0.
 */
struct buf *
getblk(dev, blkno)
	register dev_t dev;
	daddr_t blkno;
{
	register struct buf *bp, *dp;
	daddr_t dblkno;
	int s;

#ifdef DIAGNOSTIC
	if (major(dev) >= nblkdev)
		panic("blkdev");
#endif
	/*
	 * Search the cache for the block.  If we hit, but
	 * the buffer is in use for i/o, then we wait until
	 * the i/o has completed.
	 */
	dp = BUFHASH(blkno);
	dblkno = fsbtodb(blkno);
loop:
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
		if (bp->b_blkno != dblkno || bp->b_dev != dev ||
		    bp->b_flags&B_INVAL)
			continue;
		s = splbio();
		if (bp->b_flags&B_BUSY) {
			bp->b_flags |= B_WANTED;
			sleep((caddr_t)bp, PRIBIO+1);
			splx(s);
			goto loop;
		}
		splx(s);
		notavail(bp);
		return (bp);
	}
	bp = getnewbuf();
	bfree(bp);
	bremhash(bp);
	binshash(bp, dp);
	bp->b_dev = dev;
	bp->b_blkno = dblkno;
	bp->b_error = 0;
	return (bp);
}

/*
 * get an empty block,
 * not assigned to any particular device
 */
struct buf *
geteblk()
{
	register struct buf *bp, *flist;

	bp = getnewbuf();
	bp->b_flags |= B_INVAL;
	bfree(bp);
	bremhash(bp);
	flist = &bfreelist[BQ_AGE];
	binshash(bp, flist);
	bp->b_dev = (dev_t)NODEV;
	bp->b_error = 0;
	return (bp);
}

/*
 * Find a buffer which is available for use.
 * Select something from a free list.
 * Preference is to AGE list, then LRU list.
 */
struct buf *
getnewbuf()
{
	register struct buf *bp, *dp;
	int s;

loop:
	s = splbio();
	for (dp = &bfreelist[BQ_AGE]; dp > bfreelist; dp--)
		if (dp->av_forw != dp)
			break;
	if (dp == bfreelist) {		/* no free blocks */
		dp->b_flags |= B_WANTED;
		sleep((caddr_t)dp, PRIBIO+1);
		splx(s);
		goto loop;
	}
	splx(s);
	bp = dp->av_forw;
	notavail(bp);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
#ifdef NRAM > 0
	if(bp->b_flags & B_RAMREMAP) {
		register memaddr paddr;	/* click address of real buffer */
		extern memaddr bpaddr;

#ifdef DIAGNOSTIC
		if ((bp < &buf[0]) || (bp >= &buf[nbuf]))
			panic("getnewbuf: RAMREMAP bp addr");
#endif
		paddr = bpaddr + btoc(DEV_BSIZE) * (bp - buf);
		bp->b_un.b_addr = (caddr_t)(paddr << 6);
		bp->b_xmem = (paddr >> 10) & 077;
	}
#endif
	trace(TR_BRELSE);
	bp->b_flags = B_BUSY;
	return (bp);
}


/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
biowait(bp)
	register struct buf *bp;
{
	register int s;

	s = splbio();
	while ((bp->b_flags&B_DONE)==0)
		sleep((caddr_t)bp, PRIBIO);
	splx(s);
	if (!u.u_error)				/* XXX */
		u.u_error = geterror(bp);
}

/*
 * Mark I/O complete on a buffer.
 * Wake up anyone waiting for it.
 */
biodone(bp)
	register struct buf *bp;
{

	if (bp->b_flags & B_DONE)
		panic("dup biodone");
#ifdef UNIBUS_MAP
	if (bp->b_flags & (B_MAP|B_UBAREMAP))
		mapfree(bp);
#endif
	bp->b_flags |= B_DONE;
	if (bp->b_flags&B_ASYNC)
		brelse(bp);
	else {
		bp->b_flags &= ~B_WANTED;
		wakeup((caddr_t)bp);
	}
}

/*
 * Insure that no part of a specified block is in an incore buffer.
 */
blkflush(dev, blkno)
	register dev_t dev;
	daddr_t blkno;
{
	register struct buf *ep;
	struct buf *dp;
	register int s;

	dp = BUFHASH(blkno);
	blkno = fsbtodb(blkno);
loop:
	for (ep = dp->b_forw; ep != dp; ep = ep->b_forw) {
		if (ep->b_blkno != blkno || ep->b_dev != dev ||
		    (ep->b_flags&B_INVAL))
			continue;
		s = splbio();
		if (ep->b_flags&B_BUSY) {
			ep->b_flags |= B_WANTED;
			sleep((caddr_t)ep, PRIBIO+1);
			splx(s);
			goto loop;
		}
		if (ep->b_flags & B_DELWRI) {
			splx(s);
			notavail(ep);
			bwrite(ep);
			goto loop;
		}
		splx(s);
	}
}

/*
 * Make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */
bflush(dev)
	register dev_t dev;
{
	register struct buf *bp;
	register struct buf *flist;
	int s;

loop:
	s = splbio();
	for (flist = bfreelist; flist < &bfreelist[BQ_EMPTY]; flist++)
	for (bp = flist->av_forw; bp != flist; bp = bp->av_forw) {
		if ((bp->b_flags & B_DELWRI) == 0)
			continue;
		if (dev == NODEV || dev == bp->b_dev) {
			bp->b_flags |= B_ASYNC;
			notavail(bp);
			bwrite(bp);
			splx(s);
			goto loop;
		}
	}
	splx(s);
}

/*
 * Invalidate in core blocks belonging to closed or umounted filesystem
 *
 * This is not nicely done at all - the buffer ought to be removed from the
 * hash chains & have its dev/blkno fields clobbered, but unfortunately we
 * can't do that here, as it is quite possible that the block is still
 * being used for i/o. Eventually, all disc drivers should be forced to
 * have a close routine, which ought ensure that the queue is empty, then
 * properly flush the queues. Until that happy day, this suffices for
 * correctness.						... kre
 */
binval(dev)
	register dev_t dev;
{
	register struct buf *bp;
	register struct bufhd *hp;
#define dp ((struct buf *)hp)

	for (hp = bufhash; hp < &bufhash[BUFHSZ]; hp++)
		for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
			if (bp->b_dev == dev)
				bp->b_flags |= B_INVAL;
}
