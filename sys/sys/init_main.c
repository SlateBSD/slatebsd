/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/fs.h>
#include <sys/mount.h>
#include <sys/map.h>
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/vm.h>
#include <sys/clist.h>
#include <sys/reboot.h>
#include <sys/systm.h>
#include <sys/kernel.h>

/*
 * Initialization code.
 * Called from cold start routine as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	turn on clock
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 */
int
main()
{
	register struct proc *p;
	register int i;

	startup();

	/*
	 * set up system process 0 (swapper)
	 */
	p = &proc[0];
	p->p_addr = *ka6;
	p->p_stat = SRUN;
	p->p_flag |= SLOAD|SSYS;
	p->p_nice = NZERO;

	u.u_procp = p;			/* init user structure */
	u.u_ap = u.u_arg;
	u.u_lastfile = -1;
	for (i = 1; i < NGROUPS; i++)
		u.u_groups[i] = NOGROUP;
	for (i = 0; i < sizeof(u.u_rlimit)/sizeof(u.u_rlimit[0]); i++)
		u.u_rlimit[i].rlim_cur = u.u_rlimit[i].rlim_max = 
		    RLIM_INFINITY;
	/*
	 * Initialize tables, protocols, and set up well-known inodes.
	 */
	cinit();
	pqinit();
	xinit();
	ihinit();
	bhinit();
	binit();
#ifdef UNIBUS_MAP
	if (ubmap)
		(void)ubinit();
#endif
#ifdef UCB_NET
	mbinit();
	netinit();
#endif
	clkstart();
	iinit();

/* kick off timeout driven events by calling first time */
	schedcpu();

/* set up the root file system */
	rootdir = iget(rootdev, &mount[0].m_filsys, (ino_t)ROOTINO);
	iunlock(rootdir);
	u.u_cdir = iget(rootdev, &mount[0].m_filsys, (ino_t)ROOTINO);
	iunlock(u.u_cdir);
	u.u_rdir = NULL;

	/*
	 * make init process
	 */
#ifdef UCB_FRCSWAP
	idleflg = 1;			/* init can't cause swap */
#endif
	if (newproc(0)) {
		expand((int)btoc(szicode), S_DATA);
		expand((int)1, S_STACK);	/* one click of stack */
		estabur((u_int)0, (u_int)btoc(szicode), (u_int)1, 0, RO);
		copyout((caddr_t)icode, (caddr_t)0, szicode);
		/*
		 * return goes to location 0 of user init code
		 * just copied out.
		 */
		return 0;
	}
	else
		sched();
}

/*
 * Initialize hash links for buffers.
 */
static void
bhinit()
{
	register int i;
	register struct bufhd *bp;

	for (bp = bufhash, i = 0; i < BUFHSZ; i++, bp++)
		bp->b_forw = bp->b_back = (struct buf *)bp;
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */
static void
binit()
{
	register struct buf *bp;
	register int i;
	caddr_t paddr;

	for (bp = bfreelist; bp < &bfreelist[BQUEUES]; bp++)
		bp->b_forw = bp->b_back = bp->av_forw = bp->av_back = bp;
	paddr = ((long)bpaddr) << 6;
	for (i = 0; i < NBUF; i++, paddr += MAXBSIZE) {
		bp = &buf[i];
		bp->b_dev = NODEV;
		bp->b_bcount = 0;
		bp->b_un.b_addr = (caddr_t)loint(paddr);
		bp->b_xmem = hiint(paddr);
		binshash(bp, &bfreelist[BQ_AGE]);
		bp->b_flags = B_BUSY|B_INVAL;
		brelse(bp);
	}
}

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */
static void
cinit()
{
	register int ccp;
	register struct cblock *cp;

	ccp = (int)cfree;
#ifdef UCB_CLIST
	mapseg5(clststrt, clstdesc);	/* don't save, we know it's normal */
#else
	ccp = (ccp + CROUND) & ~CROUND;
#endif
	for (cp = (struct cblock *)ccp; cp <= &cfree[nclist - 1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
		cfreecount += CBSIZE;
	}
#ifdef UCB_CLIST
	normalseg5();
#endif
}

/*
 * Iinit is called once (from main) very early in initialization.
 * It reads the root's super block and initializes the current date
 * from the last modified date.
 *
 * panic: iinit -- cannot read the super block
 * (usually because of an IO error).
 */
static void
iinit()
{
	register struct bdevsw *bdp;
	register struct buf *bp;
	register struct fs *fp;

	for (bdp = bdevsw; bdp < bdevsw + nblkdev; bdp++)
		(void)(*bdp->d_root)();
	(*bdevsw[major(rootdev)].d_open)(rootdev, B_READ);
	(*bdevsw[major(swapdev)].d_open)(swapdev, B_READ);
	bp = bread(rootdev, SUPERB);
	if (u.u_error)
		panic("iinit");
	fp = &mount[0].m_filsys;
	bcopy(mapin(bp), (caddr_t)fp, sizeof(struct fs));
	mapout(bp);
	mount[0].m_inodp = (struct inode *)1;
	brelse(bp);
	mount[0].m_dev = rootdev;
	fp->fs_flock = fp->fs_ilock = fp->fs_ronly = 0;
	fp->fs_lasti = 1;
	fp->fs_nbehind = 0;
	fp->fs_fsmnt[0] = '/';
	fp->fs_fsmnt[1] = '\0';
	fp->fs_ronly = boothowto&RB_RDONLY ? 1 : 0;
	time.tv_sec = fp->fs_time;
	boottime = time;
}
