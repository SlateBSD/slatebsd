/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ufs_bmap.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "systm.h"
#include "conf.h"
#include "inode.h"
#include "user.h"
#include "buf.h"
#include "fs.h"
#include "uio.h"

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * When convenient, it also leaves the physical
 * block number of the next block of the file in rablock
 * for use in read-ahead.
 */
daddr_t
bmap(ip, bn, rwflg, clrflg)
	register struct inode *ip;
	daddr_t bn;
	int rwflg, clrflg;
{
	register int i;
	register struct buf *bp;
	struct buf *nbp;
	int j, sh;
	daddr_t nb, *bap, ra;

	if (bn < 0) {
		u.u_error = EFBIG;
		return((daddr_t)0);
	}
	ra = rablock = 0;

	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if (bn < NADDR-3) {
		i = bn;
		nb = ip->i_addr[i];
		if (nb == 0) {
			if (rwflg == B_READ || (bp = alloc(ip, clrflg)) == NULL)
				return((daddr_t)-1);
			nb = dbtofsb(bp->b_blkno);
			if ((ip->i_mode&IFMT) == IFDIR)
				/*
				 * Write directory blocks synchronously
				 * so they never appear with garbage in
				 * them on the disk.
				 */
				bwrite(bp);
			else
				bdwrite(bp);
			ip->i_addr[i] = nb;
			ip->i_flag |= IUPD|ICHG;
		}
		if (i < NADDR-4)
			rablock = ip->i_addr[i+1];
		return(nb);
	}

	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	sh = 0;
	nb = 1;
	bn -= NADDR-3;
	for (j = 3;j > 0;j--) {
		sh += NSHIFT;
		nb <<= NSHIFT;
		if (bn < nb)
			break;
		bn -= nb;
	}
	if (j == 0) {
		u.u_error = EFBIG;
		return((daddr_t)0);
	}

	/*
	 * fetch the first indirect block
	 */
	nb = ip->i_addr[NADDR-j];
	if (nb == 0) {
		if (rwflg == B_READ || (bp = alloc(ip, 1)) == NULL)
			return((daddr_t) -1);
		nb = dbtofsb(bp->b_blkno);
		/*
		 * Write synchronously so that indirect blocks
		 * never point at garbage.
		 */
		bwrite(bp);
		ip->i_addr[NADDR-j] = nb;
		ip->i_flag |= IUPD|ICHG;
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(;j <= 3;j++) {
		bp = bread(ip->i_dev, nb);
		if ((bp->b_flags & B_ERROR) || bp->b_resid) {
			brelse(bp);
			return((daddr_t)0);
		}
		bap = (daddr_t *) mapin(bp);
		sh -= NSHIFT;
		i = (bn>>sh) & NMASK;
		nb = bap[i];
		/*
		 * calculate read-ahead
		 */
		if (i < NINDIR-1)
			ra = bap[i+1];
		mapout(bp);
		if (nb == 0) {
			if (rwflg == B_READ || (nbp = alloc(ip, 1)) == NULL) {
				brelse(bp);
				return((daddr_t) -1);
			}
			nb = dbtofsb(nbp->b_blkno);
			if (j < 3 || (ip->i_mode&IFMT) == IFDIR)
				/*
				 * Write synchronously so indirect blocks
				 * never point at garbage and blocks
				 * in directories never contain garbage.
				 */
				bwrite(nbp);
			else
				bdwrite(nbp);
			bap = (daddr_t *) mapin(bp);
			bap[i] = nb;
			mapout(bp);
			bdwrite(bp);
		} else
			brelse(bp);
	}
	rablock = ra;
	return(nb);
}
