/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)machdep.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"
#include "../machine/iopage.h"

#include "inode.h"
#include "time.h"
#include "resource.h"
#include "proc.h"
#include "fs.h"
#include "map.h"
#include "buf.h"
#include "text.h"
#include "file.h"
#include "clist.h"
#include "uba.h"
#include "callout.h"
#include "reboot.h"
#include "errno.h"
#include "systm.h"
#include "ram.h"
#ifdef UCB_NET
#include "mbuf.h"
#endif

size_t	physmem;	/* total amount of physical memory (for savecore) */

#ifndef	NOKA5
segm	seg5;		/* filled in by initialization */
#endif

/*
 * Machine dependent startup code
 */
startup()
{
#ifdef UCB_CLIST
	extern memaddr clststrt;
#endif
#ifdef UCB_NET
	extern memaddr mbbase;
	extern int mbsize;
#endif
	extern ubadr_t	clstaddr;
	extern int end;
	register memaddr i, freebase, maxclick;
#if NRAM > 0
	size_t ramsize;
#endif

	printf("\n%s\n", version);

#ifdef NOKA5
	if (&end > SEG5)
		panic("_end > SEG5");
#else
	saveseg5(seg5);		/* must be done before clear() is called */
	/*
	 * REMAP_AREA is the start of possibly-mapped area, for consistency
	 * check.  Only proc, text and file tables are after it, and it must
	 * lie at <= 0120000, or other kernel data will be mapped out.
	 */
	if (REMAP_AREA > SEG5)
		panic("remapped area > SEG5");
#endif

	/*
	 * Zero and free all of core:
	 *
	 * MAXCLICK is the maximum accessible physical memory, assuming an 8K
	 * I/O page.  On systems without a Unibus map the end of memory is
	 * heralded by the beginning of the I/O page (some people have dz's
	 * at 0160000).  On systems with a Unibus map, the last 256K of the
	 * 4M address space is off limits since 017000000 to 017777777 is the
	 * actual 18 bit Unibus address space.  896 is btoc(64K - 8K), 3968
	 * is btoc(256K - 8K), 61440 is btoc(4M - 256K), and 65408 is btoc(4M
	 * - 8K).  The 16 bit entry included only for completeness, don't use
	 * it.
	 *
	 * If we're not on a UNIBUS machine and Q22 isn't defined we
	 * artificially limit ourselves to 256K-8K to avoid problems of
	 * 18-bit DMA disk or tape peripherals attached to 22-bit Q-BUS
	 * machines.  See extended notes in /sys/conf/GENERIC.
	 */
#define MAXCLICK_16	896		/* 16 bit UNIBUS or QBUS */
#define MAXCLICK_18	3968		/* 18 bit UNIBUS or QBUS */
#define MAXCLICK_22U	61440		/* 22 bit UNIBUS (UNIBUS mapping) */
#define MAXCLICK_22	65408		/* 22 bit QBUS */

#ifdef Q22
	maxclick = ubmap ? MAXCLICK_22U : MAXCLICK_22;
#else
	maxclick = ubmap ? MAXCLICK_22U : MAXCLICK_18;
#endif
	i = freebase = *ka6 + USIZE;
	UISD[0] = ((stoc(1) - 1) << 8) | RW;
	for (;;) {
		UISA[0] = i;
		if (fuibyte((caddr_t)0) < 0)
			break;
		++maxmem;
		/* avoid testing locations past "real" memory. */
		if (++i >= maxclick)
			break;
	}
	clear(freebase,i - freebase);
	mem_parity();			/* enable parity checking */
	clear(freebase,i - freebase);	/* quick check for parities */
	mfree(coremap,i - freebase,freebase);
	physmem = i;

	procNPROC = proc + nproc;
	textNTEXT = text + ntext;
	inodeNINODE = inode + ninode;
	fileNFILE = file + nfile;

	/*
	 * IMPORTANT! Mapped out clists should always be allocated first!
	 * This prevents needlessly having to restrict all memory use
	 * (maxclick) to 248K just because an 18-bit DH is present on a
	 * 22-bit Q-BUS machine.  The maximum possible location for mapped
	 * out clists this way is 232K (56K base + 15 * 8K overlays + 48K
	 * data space + 8K (maximum) user structure, which puts the maximum
	 * top of mapped out clists at 240K ...
	 */
#ifdef UCB_CLIST
#define C	(nclist * sizeof(struct cblock))
	if ((clststrt = malloc(coremap, btoc(C))) == 0)
		panic("clists");
	maxmem -= btoc(C);
	clstaddr = ((ubadr_t)clststrt) << 6;
#undef C
#else
	clstaddr = (ubadr_t)cfree;
#endif

#define B	(size_t)(((long)nbuf * (MAXBSIZE)) / ctob(1))
	if ((bpaddr = malloc(coremap, B)) == 0)
		panic("buffers");
	maxmem -= B;
#undef B

#if defined(PROFILE) && !defined(ENABLE34)
	maxmem -= msprof();
#endif

#ifdef UCB_NET
	if (!(mbbase = malloc(coremap, btoc(mbsize))))
		panic("mbbase");
	maxmem -= btoc(mbsize);
#endif

#if NRAM > 0
	ramsize = raminit();
	maxmem -= ramsize;
#endif

	if (MAXMEM < maxmem)
		maxmem = MAXMEM;

	printf("phys mem  = %D\n", ctob((long)physmem));
	printf("avail mem = %D\n", ctob((long)maxmem));
	printf("user mem  = %D\n", ctob((long)maxmem));
#if NRAM > 0
	printf("ram disk  = %D\n", ctob((long)ramsize));
#endif
#ifdef DIAGNOSTIC
	printf("%d procs (%d bytes)\n",nproc,nproc * sizeof(struct proc));
	printf("%d texts (%d bytes)\n",ntext,ntext * sizeof(struct text));
	printf("%d inodes (%d bytes)\n",ninode,ninode * sizeof(struct inode));
	printf("%d files (%d bytes)\n",nfile,nfile * sizeof(struct file));
	printf("%d buffers (%D bytes)\n",nbuf,(long)nbuf * MAXBSIZE);
	printf("%d clists (%d bytes)\n",nclist,nclist * sizeof(struct cblock));
#ifdef UCB_NET
	printf("%d mbufs %d cached (%d bytes)\n", NMBUFS, NMBCACHE, mbsize);
#endif
#endif
	printf("\n");

	/*
	 * free up the swap map; the decrement is because you can't put
	 * zero into a resource map, therefore we offset everything by
	 * one.
	 */
	mfree(swapmap, nswap, (memaddr)1);
	swplo--;

	/*
	 * Initialize callouts
	 */
	callfree = callout;
	for (i = 1; i < ncallout; i++)
		callout[i-1].c_next = &callout[i];

	UISA[7] = ka6[1];			/* io segment */
	UISD[7] = ((stoc(1) - 1) << 8) | RW;
}

mem_parity()
{
	register int cnt;

	for (cnt = 0;cnt < 16;++cnt) {
		if (fioword((caddr_t)(MEMSYSMCR+cnt)) == -1)
			return;
		*(MEMSYSMCR+cnt) = MEMMCR_EIE;	/* enable parity interrupts */
	}
}

#if defined(PROFILE) && !defined(ENABLE34)
/*
 * Allocate memory for system profiling.  Called once at boot time.
 * Returns number of clicks used by profiling.
 *
 * The system profiler uses supervisor I space registers 2 and 3
 * (virtual addresses 040000 through 0100000) to hold the profile.
 */
msprof()
{
	memaddr proloc;
	int nproclicks;

	nproclicks = btoc(8192*2);
	proloc = malloc(coremap, nproclicks);
	if (proloc == 0)
		panic("msprof");

	*SISA2 = proloc;
	*SISA3 = proloc + btoc(8192);
	*SISD2 = 077400|RW;
	*SISD3 = 077400|RW;
	*SISD0 = RW;
	*SISD1 = RW;

	/*
	 * Enable system profiling.  Zero out the profile buffer
	 * and then turn the clock (KW11-P) on.
	 */
	clear(proloc, nproclicks);
	isprof();
	printf("profiling on\n");

	return (nproclicks);
}
#endif

#ifdef UNIBUS_MAP

bool_t	ubmap;

/*
 * Re-initialize the Unibus map registers to statically map
 * the clists and buffers.  Free the remaining registers for
 * physical I/O.
 */
ubinit()
{
	register int i, ub_nreg;
	long paddr;
#ifdef UCB_NET
	extern int ub_inited;
#endif

	if (!ubmap)
		return;
#ifdef UCB_NET
	++ub_inited;
#endif
	/*
	 * Clists start at UNIBUS virtual address 0.  The size of
	 * the clist segment can be no larger than UBPAGE bytes.
	 * Clstaddt was the physical address of clists.
	 */
	if (nclist * sizeof(struct cblock) > ctob(stoc(1)))
		panic("clist area too large");
	setubregno(0, clstaddr);
	clstaddr = (ubadr_t)0;

	/*
	 * Buffers start at UNIBUS virtual address BUF_UBADDR.
	 */
	paddr = ((long)bpaddr) << 6;
	ub_nreg = nubreg(nbuf, MAXBSIZE);
	for (i = BUF_UBADDR/UBPAGE; i < ub_nreg + (BUF_UBADDR/UBPAGE); i++) {
		setubregno(i, paddr);
		paddr += (long)UBPAGE;
	}
	mfree(ub_map, 31 - ub_nreg - 1, 1 + ub_nreg);
}
#endif

int waittime = -1;

boot(dev, howto)
	register dev_t dev;
	register int howto;
{
	register struct fs *fp;

	/*
	 * Force the root filesystem's superblock to be updated,
	 * so the date will be as current as possible after
	 * rebooting.
	 */
	if (fp = getfs(rootdev))
		fp->fs_fmod = 1;
	if ((howto&RB_NOSYNC)==0 && waittime < 0 && bfreelist[0].b_forw) {
		waittime = 0;
		printf("syncing disks... ");
		(void) splnet();
		/*
		 * Release inodes held by texts before update.
		 */
		xumount(NODEV);
		update();
		{ register struct buf *bp;
		  int iter, nbusy;

		  for (iter = 0; iter < 20; iter++) {
			nbusy = 0;
			for (bp = &buf[nbuf]; --bp >= buf; )
				if (bp->b_flags & B_BUSY)
					nbusy++;
			if (nbusy == 0)
				break;
			printf("%d ", nbusy);
			DELAY(40000L * iter);
		  }
		}
		printf("done\n");
	}
	(void)splhigh();
	if (howto & RB_HALT) {
		printf("halting\n");
		halt();
		/*NOTREACHED*/
	}
	else {
		if (howto & RB_DUMP) {
			/*
			 * save the registers in low core.
			 */
			saveregs();
			dumpsys();
		}
		doboot(dev, howto);
		/*NOTREACHED*/
	}
}

/*
 * Dumpsys takes a dump of memory by calling (*dump)(), which must
 * correspond to dumpdev.  *(dump)() should dump from dumplo blocks
 * to the end of memory or to the end of the logical device.
 */
dumpsys()
{
	extern int (*dump)();
	register int error;

	if (dumpdev != NODEV) {
		printf("\ndumping to dev %o, offset %D\ndump ",dumpdev,dumplo);
		error = (*dump)(dumpdev);
		switch(error) {

		case EFAULT:
			printf("device not ready (EFAULT)\n");
			break;
		case EINVAL:
			printf("arguments invalid (EINVAL)\n");
			break;
		case EIO:
			printf("I/O error (EIO)\n");
			break;
		default:
			printf("unknown error (%d)\n",error);
			break;
		case 0:
			printf("succeeded\n");
			break;
		}
	}
}
