/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)vm_swp.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "user.h"
#include "proc.h"
#include "buf.h"
#include "conf.h"
#include "systm.h"
#include "vm.h"
#include "trace.h"

/*
 * swap IO headers.  They are filled in to point
 * at the desired IO operation.
 */
struct	buf swbuf1, swbuf2;

/*
 * swap I/O
 */
swap(blkno, coreaddr, count, rdflg)
	memaddr blkno, coreaddr;
	register int count;
	int rdflg;
{
	register struct buf *bp;
	register int tcount;
	int s;

#ifdef UCB_METER
	if (rdflg) {
		cnt.v_pswpin += count;
		cnt.v_pgin++;
	}
	else {
		cnt.v_pswpout += count;
		cnt.v_pgout++;
	}
#endif
	bp = &swbuf1;
	if (bp->b_flags & B_BUSY)
		if ((swbuf2.b_flags&B_WANTED) == 0)
			bp = &swbuf2;
	s = splbio();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PSWP+1);
	}
	splx(s);
	while (count) {
		bp->b_flags = B_BUSY | B_PHYS | rdflg;
		bp->b_dev = swapdev;
		tcount = count;
		if (tcount >= 01700)	/* prevent byte-count wrap */
			tcount = 01700;
		bp->b_bcount = ctob(tcount);
		bp->b_blkno = swplo + blkno;
		bp->b_un.b_addr = (caddr_t)(coreaddr<<6);
		bp->b_xmem = (coreaddr>>10) & 077;
		trace(TR_SWAPIO);
		(*bdevsw[major(swapdev)].d_strategy)(bp);
		s = splbio();
		while((bp->b_flags&B_DONE)==0)
			sleep((caddr_t)bp, PSWP);
		splx(s);
		if ((bp->b_flags & B_ERROR) || bp->b_resid)
			panic("hard IO err in swap");
		count -= tcount;
		coreaddr += tcount;
		blkno += ctod(tcount);
	}
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	bp->b_flags &= ~(B_BUSY|B_WANTED);
}

/*
 * If rout == 0 then killed on swap error, else
 * rout is the name of the routine where we ran out of
 * swap space.
 */
swkill(p, rout)
	register struct proc *p;
	register char *rout;
{

	printf("pid %d: %s\n", p->p_pid, rout);
	uprintf("sorry, pid %d was killed in %s\n", p->p_pid, rout);
	/*
	 * To be sure no looping (e.g. in vmsched trying to
	 * swap out) mark process locked in core (as though
	 * done by user) after killing it so noone will try
	 * to swap it out.
	 */
	psignal(p, SIGKILL);
	p->p_flag |= SULOCK;
}

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 *
 * physio broken into smaller routines, 3/81 mjk
 *	chkphys(WORD or BYTE) checks validity of word- or byte-
 *	oriented transfer (for physio or device drivers);
 *	physbuf(strat,bp,rw) fills in the buffer header.
 *
 * physio divided into two functions, 1/83 - Mike Edmonds - Tektronix
 *	Physio divided into separate functions:
 *		physio (for WORD i/o)
 *		bphysio (for BYTE i/o)
 *	This allows byte-oriented devices (such as tape drives)
 *	to write/read odd length blocks.
 *
 * since physio/bphysio just called physio1 with BYTE or WORD added
 *	to the argument list, adjusted all calls to physio/bphysio
 *	to pass the correct argument themselves.
 *		5/86 kb
 */
physio(strat, bp, dev, rw, kind)
	int (*strat)();
	register struct buf *bp;
	dev_t dev;
	int rw, kind;
{
	register int error, s;

	error = chkphys(kind);
	if (error)
		return(error);
	physbuf(bp, dev, rw);
	u.u_procp->p_flag |= SLOCK;
	(*strat)(bp);
	s = splbio();
	while ((bp->b_flags&B_DONE)==0)
		sleep((caddr_t)bp, PRIBIO);
	splx(s);
	error = geterror(bp);
	u.u_procp->p_flag &= ~SLOCK;
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	bp->b_flags &= ~(B_BUSY|B_WANTED);
	u.u_count = bp->b_resid;
	return(error);
}

/*
 * check for validity of physical I/O area
 * (modified from physio to use flag for BYTE-oriented transfers)
 */
chkphys(flag)
	int flag;
{
	register u_int base;
	register int nb, ts;

	base = (u_int)u.u_base;
	/*
	 * Check odd base, odd count, and address wraparound
	 * Odd base and count not allowed if flag = WORD,
	 * allowed if flag = BYTE.
	 */
	if (flag == WORD && ((base|u.u_count) & 01))
		return(EFAULT);
	if (base >= base + u.u_count)
		return(EFAULT);
	if (u.u_sep)
		ts = 0;
	else
		ts = (u.u_tsize + 127) & ~0177;
	nb = (base >> 6) & 01777;
	/*
	 * Check overlap with text. (ts and nb now
	 * in 64-byte clicks)
	 */
	if (nb < ts)
		return(EFAULT);
	/*
	 * Check that transfer is either entirely in the
	 * data or in the stack: that is, either
	 * the end is in the data or the start is in the stack
	 * (remember wraparound was already checked).
	 */
	if ((((base + u.u_count) >> 6) & 01777) >= ts + u.u_dsize &&
	    nb < 1024 - u.u_ssize)
		return(EFAULT);
	return(0);
}

/*
 * wait for buffer header, then fill it in to do physical I/O.
 */
physbuf(bp,dev,rw)
	register struct buf *bp;
	dev_t dev;
	int rw;
{
	{
		register int s;

		s = splbio();
		while (bp->b_flags&B_BUSY) {
			bp->b_flags |= B_WANTED;
			sleep((caddr_t)bp, PRIBIO+1);
		}
		splx(s);
	}
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
	/*
	 * Compute physical address by simulating
	 * the segmentation hardware.
	 */
	{
		register u_int base;
		register int ts;
		int nb;

		base = (u_int)u.u_base;
		nb = (base >> 6) & 01777;
		ts = (u.u_sep ? UDSA: UISA)[nb >> 7] + (nb & 0177);
		bp->b_un.b_addr = (caddr_t)((ts << 6) + (base & 077));
		bp->b_xmem = (ts >> 10) & 077;
		bp->b_blkno = u.u_offset >> PGSHIFT;
		bp->b_bcount = u.u_count;
		bp->b_error = 0;
	}
}
