/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ra.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

 /***********************************************************************
 *									*
 *			Copyright (c) 1983 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 ***********************************************************************/

/* 
 * uda.c - UDA50A Driver
 * 
 * Date:        Jan  30 1984
 *
 * This thing has been beaten beyound belief.  It still has two main problems:
 *
 * When this device is on the same unibus as another DMA device
 * like a versatec or a rk07, the Udstrat routine complains that it still
 * has a buffered data path that it shouldn't.  I don't know why.
 *
 * decvax!rich.
 *
 */

/*
 * RA MSCP disk device driver
 *
 * Should work with the following (at least!):
 *	RUX1  (RX50)
 * 	RQDX? (RD51, RD52, RD53)
 *	UDA50 (RA60, RA80, RA81)
 *	KLESI (RC25)
 *
 * Tim Tucker, Gould Electronics, Sep 1985
 * Note:  This driver is based on the UDA50 4.3 BSD source.
 */

#include "ra.h"
#if 	NRAD > 0  &&  NRAC > 0
#include "param.h"
#include "../machine/seg.h"
#include "../machine/mscp.h"

#include "systm.h"
#include "buf.h"
#include "conf.h"
#include "map.h"
#include "uba.h"
#include "rareg.h"
#include "dk.h"
#include "errno.h"

#ifndef KDSA0
#define	KDSA0				((u_short *)0172360)
#endif
#define	RACON(x)			((minor(x) >> 6) & 03)
#define	RAUNIT(x)			((minor(x) >> 3) & 07)

struct	rasizes	{
	daddr_t nblocks;
	daddr_t blkoff;
}  ra25_sizes[8] = {
	15884,	0,		/* A=blk 0 thru 15883 */
	10032,	15884,		/* B=blk 15884 thru 25915 */
	-1,	25916,		/* C=blk 49324 thru 50901 */
	0,	0,		/* D=unused */
	0,	0,		/* E=unused */
	0,	0,		/* F=unused */
	0,	0,		/* G=unused */
	-1,	0,		/* H=blk 0 thru end */
}, rd52_sizes[8] = { 		/* Setup for RX50, RD51, RD52 and RD53 disks */
	9700,	0,		/* A=blk 0 thru 9699 (root for 52 & 53) */
	17300,	9700,		/* B=blk 9700 thru 26999 (52 & 53 only) */
	3100,	27000,		/* C=blk 27000 thru 30099 (swap 52 & 53) */
	-1,	30100,		/* D=blk 30100 thru end (52 & 53 only) */
	7460,	0,		/* E=blk 0 thru 7459 (root 51) */
	2240,	7460,		/* F=blk 7460 thru 9699 (swap 51) */
	-1,	9700,		/* G=blk 9700 thru end (51, 52 & 53) */
	-1,	0,		/* H=blk 0 thru end (51, 52, 53 & RX50) */
}, ra60_sizes[8] = {
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	193282,	49324,		/* C=blk 49324 thru 242605 */
	15884,	242606,		/* D=blk 242606 thru 258489 */
	-1,	258490,		/* E=blk 258490 thru end */
	0,	0,		/* F=unused */
	-1,	242606,		/* G=blk 242606 thru end */
	-1,	0,		/* H=blk 0 thru end */
}, ra80_sizes[8] = {
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	49324,		/* C=blk 49324 thru end */
	0,	0,		/* D=unused */
	0,	0,		/* E=unused */
	0,	0,		/* F=unused */
	0,	0,		/* G=unused */
	-1,	0,		/* H=blk 0 thru end */
}, ra81_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	193282,	49324,		/* C=blk 49324 thru 242605 */
	15884,	242606,		/* D=blk 242606 thru 258489 */
	307200,	258490,		/* E=blk 258490 thru 565689 */
	-1,	565690,		/* F=blk 565690 thru end */
	-1,	242606,		/* G=blk 242606 thru end */
	-1,	0,		/* H=blk 0 thru end */
};

#define RC_RCT	102		/* rc25 rct area */
#define RD_RCT	32		/* # of sectors in remap area (don't touch) */
#define RA_RCT	1000		/* Big ra disk rct area */
/*------------------------------------------------------------------------*/

#define	NRSPL2	3		/* log2 number of response packets */
#define	NCMDL2	3		/* log2 number of command packets */
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)

typedef	struct	{		/* Swap shorts for MSCP controller! */
	short	lsh;
	short	hsh;
} Trl;

/*
 * RA Communications Area
 */
struct  raca {
	short	ca_xxx1;	/* unused */
	char	ca_xxx2;	/* unused */
	char	ca_bdp;		/* BDP to purge */
	short	ca_cmdint;	/* command queue transition interrupt flag */
	short	ca_rspint;	/* response queue transition interrupt flag */
	Trl	ca_rsp[NRSP];	/* response descriptors */
	Trl	ca_cmd[NCMD];	/* command descriptors */
};

#define	RINGBASE	(4 * sizeof(short))

#define	RA_OWN	0x8000	/* Controller owns this descriptor */
#define	RA_INT	0x4000	/* allow interrupt on ring transition */

typedef	struct {
	struct raca	ra_ca;		/* communications area */
	struct mscp	ra_rsp[NRSP];	/* response packets */
	struct mscp	ra_cmd[NCMD];	/* command packets */
} ra_comT;

typedef	struct	ra_info	{
	struct  rasizes	*ra_size;	/* Partion tables for drive */
	daddr_t		ra_dsize;	/* Max user size from online pkt */
	short		ra_unit;	/* controller unit # */
	struct	buf	ra_dtab;	/* I/O disk drive queues */
	struct	buf	ra_rtab;	/* raw I/O disk block header */
} ra_infoT;

typedef	struct	{
	radeviceT 	*RAADDR;	/* Controller bus address */
	short		sc_unit;	/* attach unit # */
	short		sc_state;	/* state of controller */
	short		sc_ivec;	/* interrupt vector address */
	short		sc_credits;	/* transfer credits */
	short		sc_lastcmd;	/* pointer into command ring */
	short		sc_lastrsp;	/* pointer into response ring */
	struct	buf	sc_ctab;	/* Controller queue */
	struct	buf	sc_wtab;	/* I/O wait queue, for controller */
	short		sc_cp_wait;	/* Command packet wait flag */
	ra_comT		*sc_com;	/* Communications area pointer */
	ra_infoT	*sc_drives[8];	/* Disk drive info blocks */
} ra_softcT;

ra_softcT		ra_sc[NRAC];	/* Controller table */
ra_comT			ra_com[NRAC];	/* Communications area table */
ra_infoT		ra_disks[NRAD];	/* Disk table */
static	struct	buf	racomphys;	/* Communications area phys map */
struct	buf		ratab;		/* Interface queue (for bio) */

#ifdef UCB_METER
static	int		ra_dkn = -1;	/* number for iostat */
#endif

/*
 * Controller states
 */
#define	S_IDLE	0		/* hasn't been initialized */
#define	S_STEP1	1		/* doing step 1 init */
#define	S_STEP2	2		/* doing step 2 init */
#define	S_STEP3	3		/* doing step 3 init */
#define	S_SCHAR	4		/* doing "set controller characteristics" */
#define	S_RUN	5		/* running */

#ifdef RADEBUG
#define PRINTD(x)	printf x 
#else
#define PRINTD(x)
#endif

#ifdef RABUGDUMP
#define PRINTB(x)	printf x 
#else
#define PRINTB(x)
#endif

int		raattach(), raintr();
int		wakeup();
long		raphys();
struct	mscp 	*ragetcp();

#define	b_qsize	b_resid		/* queue size per drive, in rqdtab */

/*
 * Setup root MSCP device (use standard address 0172150).
 */
raroot()
{
	raattach((radeviceT *)0172150, 0);
}

/*
 * Attach controller for autoconfig system.
 */
raattach(addr, unit)
	radeviceT			*addr;
	int				unit;
{
	register	ra_softcT	*sc = &ra_sc[unit];

#ifdef UCB_METER
	if (ra_dkn < 0)
		dk_alloc(&ra_dkn, NRAD, "ra", 60L * 31L * 256L);
#endif

	/* Check for bad address (no such controller) */
	if (sc->RAADDR == NULL && addr != NULL) {
		sc->RAADDR = addr;
		sc->sc_unit = unit;
		sc->sc_com = &ra_com[unit];
		return(1);
	}

	/*
	 * Binit and autoconfig both attempt to attach unit zero if ra is
	 * rootdev
	 */
	return(unit ? 0 : 1);
}

/*
 * Return a pointer to a free disk table entry
 */
ra_infoT *
ragetdd()
{
	register	int		i;
	register	ra_infoT	*p;

	for (i = NRAD, p = ra_disks; i--; p++)
		if (p->ra_dsize == 0L)
			return(p);
	return(NULL);
}

/*
 * Open a RA.  Initialize the device and set the unit online.
 */
raopen(dev, flag)
	dev_t 				dev;
	int 				flag;
{
	register	ra_infoT	*disk;
	register	struct	mscp	*mp;
	register	ra_softcT	*sc = &ra_sc[RACON(dev)];
	int				unit = RAUNIT(dev);
	int				s, i;

	PRINTD(("raopen: dev=%x, flags=%d\n", dev, flag));

	/* Check that controller exists */
	if (sc->RAADDR == NULL)
		return(ENXIO);

	/* Open device */
	if (sc->sc_state != S_RUN) {
		s = spl5();

		/* initialize controller if idle */
		if (sc->sc_state == S_IDLE) {
			if (rainit(sc)) {
				splx(s);
				return(ENXIO);
			}
		}

		/* wait for initialization to complete */
		timeout(wakeup, (caddr_t)&sc->sc_ctab, 12 * LINEHZ);
		sleep((caddr_t)&sc->sc_ctab, PSWP+1);
		if (sc->sc_state != S_RUN) {
			splx(s);
			return(EIO);
		}
		splx(s);
	}

	/*
	 * Check to see if the device is really there.  This code was
	 * taken from Fred Canters 11 driver.
	 */
	if ((disk = sc->sc_drives[unit]) == NULL) {
		PRINTD(("raopen: opening new disk %d\n", unit));
		s = spl5();
		while ((mp = ragetcp(sc)) == 0) {
			++sc->sc_cp_wait;
			sleep(&sc->sc_cp_wait, PSWP+1);
			--sc->sc_cp_wait;
		}
		/* Allocate disk table entry for disk */
		if ((disk = ragetdd()) != NULL) {
			sc->sc_drives[unit] = disk;
			disk->ra_unit = unit;
			disk->ra_dsize = -1L;
		} else {
			printf("ra%d: out of disk data structures!\n", unit);
			splx(s);
			return(ENXIO);
		}

		/* Try to online disk unit */
		mp->m_opcode = M_O_ONLIN;
		mp->m_unit = unit;
		mp->m_cmdref = (unsigned)&disk->ra_dsize;
		((Trl *)mp->m_dscptr)->hsh |= RA_OWN|RA_INT;
		i = sc->RAADDR->raip;
		timeout(wakeup, (caddr_t)mp->m_cmdref, 10 * LINEHZ);
		sleep((caddr_t)mp->m_cmdref, PSWP+1);
		splx(s);
	}

	/* Did it go online? */
	if (disk->ra_dsize == -1L) {
		PRINTD(("raopen: disk didn't go online\n"));
		s = spl5();
		disk->ra_dsize = 0L;
		sc->sc_drives[unit] = NULL;
		splx(s);
		return(ENXIO);
	}
	PRINTD(("raopen: disk online\n"));

	return(0);
}

/*
 * Initialize a RA.  Initialize data structures, and start hardware
 * initialization sequence.
 */
rainit(sc)
	register	ra_softcT	*sc;
{
	long				racomaddr;

	/*
	 * Cold init of controller
	 */
	sc->sc_ivec = RA_VECTOR + sc->sc_unit * 04;
	++sc->sc_ctab.b_active;
	PRINTD(("rainit: unit=%d, vec=%o\n", sc->sc_unit, sc->sc_ivec));

	/*
	 * Get physical address of RINGBASE
	 */
	if (racomphys.b_flags == 0) {
		racomphys.b_un.b_addr = (caddr_t)loint(raphys((u_int)ra_com));
		racomphys.b_xmem = hiint(raphys((u_int)ra_com));
		racomphys.b_bcount = sizeof(ra_com);
		racomphys.b_flags = B_PHYS;
#ifdef UNIBUS_MAP
		mapalloc(&racomphys);
#endif
		PRINTD(("rainit: racomphys b_addr=%o, b_xmem=%o\n",
			racomphys.b_un.b_addr, racomphys.b_xmem));
	}
	racomaddr =
		((u_int)racomphys.b_un.b_addr | ((long)racomphys.b_xmem << 16))
		+ sizeof(ra_comT) * sc->sc_unit
		+ (u_int)RINGBASE;

	/*
	 * Get individual controller RINGBASE physical address 
	 */
	sc->sc_ctab.b_un.b_addr = (caddr_t)loint(racomaddr);
	sc->sc_ctab.b_xmem = hiint(racomaddr);
	PRINTD(("rainit: com area addr low=0%o, high=0%o\n",
		sc->sc_ctab.b_un.b_addr, sc->sc_ctab.b_xmem));

	/*
	 * Start the hardware initialization sequence.
	 */
	sc->RAADDR->raip = 0;
	while ((sc->RAADDR->rasa & RA_STEP1) == 0)
		if (sc->RAADDR->rasa & RA_ERR)
			return(1);
	sc->RAADDR->rasa = RA_ERR | (NCMDL2 << 11) | (NRSPL2 << 8) | RA_IE
			| (sc->sc_ivec / 4);

	/*
	 * Initialization continues in interrupt routine.
	 */
	sc->sc_state = S_STEP1;
	sc->sc_credits = 0;
	return(0);
}

/*
 * Return the physical address corresponding to a virtual data space address.
 * On a separate I&D CPU this is a noop, but it's only called when the first
 * controller is initialized and on a dump.
 */
long
raphys(vaddr)
	register	unsigned	vaddr;
{
	register	unsigned	click;

	click = (sep_id ? KDSA0 : KISA0)[(vaddr >> 13) & 07];
	return(((long)click << 6) + (vaddr & 017777));
}

rastrategy(bp)
	register struct	buf 		*bp;
{
	register ra_infoT		*disk;
	register struct buf 		*dp;
	ra_softcT			*sc = &ra_sc[RACON(bp->b_dev)];
	int 				part = minor(bp->b_dev) & 07;
	daddr_t 			sz, maxsz;
	int 				s;

	/* Is disk online */
	if ((disk = sc->sc_drives[dkunit(bp)]) == NULL || disk->ra_dsize <= 0L)
		goto bad;

	/* Valid block in device partition */
	sz = (bp->b_bcount + 511) >> 9;
	if ((maxsz = disk->ra_size[part].nblocks) < 0)
		maxsz = disk->ra_dsize - disk->ra_size[part].blkoff;
	if (bp->b_blkno < 0 || bp->b_blkno + sz > maxsz
	    || disk->ra_size[part].blkoff >= disk->ra_dsize)
		goto bad;

#ifdef UNIBUS_MAP
	/* Get unibus map if we need it */
	mapalloc(bp);
#endif

	/*
	 * Link the buffer onto the drive queue
	 */
	s = spl5();
	dp = &disk->ra_dtab;
	if (dp->b_actf == 0)
		dp->b_actf = bp;
	else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
	bp->av_forw = 0;

	/*
	 * Link the drive onto the controller queue
	 */
	if (dp->b_active == 0) {
		dp->b_forw = NULL;
		if (sc->sc_ctab.b_actf == NULL)
			sc->sc_ctab.b_actf = dp;
		else
			sc->sc_ctab.b_actl->b_forw = dp;
		sc->sc_ctab.b_actl = dp;
		dp->b_active = 1;
	}

	/*
	 * Start controller if idle.
	 */
	if (sc->sc_ctab.b_active == 0) {
		rastart(sc);
	}

	splx(s);
	return;

bad:
	bp->b_flags |= B_ERROR;
	iodone(bp);
	return;
}

ra_infoT	*
rarawcheck(dev)
	register	dev_t 		dev;
{
	register	ra_softcT	*sc;
	register	ra_infoT	*disk;

	/* Check controller and disk unit */
	if (RACON(dev) >= NRAC || (sc = &ra_sc[RACON(dev)])->RAADDR == NULL
	    || (disk = sc->sc_drives[RAUNIT(dev)]) == NULL
	    || disk->ra_dsize <= 0L)
		return(NULL);

	return(disk);
}

raread(dev)
	dev_t 				dev;
{
	register	ra_infoT	*disk;

	/* Check controller and disk unit */
	if ((disk = rarawcheck(dev)) == NULL)
		return(ENXIO);

	return(physio(rastrategy, &disk->ra_rtab, dev, B_READ, WORD));
}

rawrite(dev)
	dev_t 				dev;
{
	register	ra_infoT	*disk;

	/* Check controller and disk unit */
	if ((disk = rarawcheck(dev)) == NULL)
		return(ENXIO);

	return(physio(rastrategy, &disk->ra_rtab, dev, B_WRITE, WORD));
}

/* Start i/o, must be called at level spl5 */
rastart(sc)
	register ra_softcT		*sc;
{
	register struct mscp		*mp;
	register struct buf		*bp;
	struct buf			*dp;
	ra_infoT			*disk;
	int 				i;
	long				temp;

loop:
	/* 
	 * Anything left to do on this controller?
	 */
	if ((dp = sc->sc_ctab.b_actf) == NULL) {
		sc->sc_ctab.b_active = 0;

		/*
		 * Check for response ring transitions lost in race
		 * condition
		 */
		rarspring(sc);
		return(0);
	}

	/* Get first request waiting on queue */
	if ((bp = dp->b_actf) == NULL) {
		/*
		 * No more requests for this drive, remove
		 * from controller queue and look at next drive.
		 * We know we're at the head of the controller queue.
		 */
		dp->b_active = 0;
		sc->sc_ctab.b_actf = dp->b_forw;
		goto loop;
	}

	++sc->sc_ctab.b_active;
	if (sc->RAADDR->rasa & RA_ERR || sc->sc_state != S_RUN) {
		harderr(bp, "ra");
		printf("rasa %o, state %d\n", sc->RAADDR->rasa,
			sc->sc_state);
		/* Should requeue outstanding requests somehow */
		rainit(sc);
		return(0);
	}

	/* Issue command */
	if ((mp = ragetcp(sc)) == NULL)
		return(0);
	mp->m_cmdref = (unsigned)bp;	/* pointer to get back */
	mp->m_opcode = bp->b_flags & B_READ ? M_O_READ : M_O_WRITE;
	mp->m_unit = dkunit(bp);
	disk = sc->sc_drives[mp->m_unit];
	temp = bp->b_blkno + disk->ra_size[minor(bp->b_dev) & 7].blkoff;
	mp->m_lbn_l = loint(temp);
	mp->m_lbn_h = hiint(temp);
	mp->m_bytecnt = bp->b_bcount;
	mp->m_buf_l = (u_short)bp->b_un.b_addr;
	mp->m_buf_h = bp->b_xmem;
	PRINTD(("ra: unit=%d op=0%o lbn=%d,%d len=%d buf=0%o,0%o\n",
		mp->m_unit, mp->m_opcode, mp->m_lbn_h, mp->m_lbn_l,
		mp->m_bytecnt, mp->m_buf_h, mp->m_buf_l));
	((Trl *)mp->m_dscptr)->hsh |= RA_OWN|RA_INT;
	if (sc->RAADDR->rasa & RA_ERR)
		printf("ra: Error %d\n", sc->RAADDR->rasa);
	i = sc->RAADDR->raip;		/* initiate polling */

#ifdef UCB_METER
	if (ra_dkn >= 0) {
		int dkn = ra_dkn + mp->m_unit;

		/* Messy, should do something better than this.  Ideas? */
		++dp->b_qsize;
		dk_busy |= 1<<dkn;
		dk_xfer[dkn]++;
		dk_wds[dkn] += bp->b_bcount>>6;
	}
#endif

	/*
	 * Move drive to the end of the controller queue
	 */
	if (dp->b_forw != NULL) {
		sc->sc_ctab.b_actf = dp->b_forw;
		sc->sc_ctab.b_actl->b_forw = dp;
		sc->sc_ctab.b_actl = dp;
		dp->b_forw = NULL;
	}

	/*
	 * Move buffer to I/O wait queue
	 */
	dp->b_actf = bp->av_forw;
	dp = &sc->sc_wtab;
	bp->av_forw = dp;
	bp->av_back = dp->av_back;
	dp->av_back->av_forw = bp;
	dp->av_back = bp;
	goto loop;
}

/*
 * RA interrupt routine.
 */
raintr(unit)
	int				unit;
{
	register	ra_softcT	*sc = &ra_sc[unit];
	register	struct	mscp	*mp;
	register	struct	buf	*bp;
	u_int				i;

	PRINTD(("raintr%d: state %d, rasa %o\n", unit, sc->sc_state,
		sc->RAADDR->rasa));

	switch (sc->sc_state) {
	case S_STEP1:
#define	STEP1MASK	0174377
#define	STEP1GOOD	(RA_STEP2|RA_IE|(NCMDL2<<3)|NRSPL2)
		if ((sc->RAADDR->rasa & STEP1MASK) != STEP1GOOD) {
			sc->sc_state = S_IDLE;
			sc->sc_ctab.b_active = 0;
			wakeup((caddr_t)&sc->sc_ctab);
			return;
		}
		sc->RAADDR->rasa = (short)sc->sc_ctab.b_un.b_addr;
		sc->sc_state = S_STEP2;
		return;

	case S_STEP2:
#define	STEP2MASK	0174377
#define	STEP2GOOD	(RA_STEP3|RA_IE|(sc->sc_ivec/4))
		if ((sc->RAADDR->rasa & STEP2MASK) != STEP2GOOD) {
			sc->sc_state = S_IDLE;
			sc->sc_ctab.b_active = 0;
			wakeup((caddr_t)&sc->sc_ctab);
			return;
		}
		sc->RAADDR->rasa = sc->sc_ctab.b_xmem;
		sc->sc_state = S_STEP3;
		return;

	case S_STEP3:
#define	STEP3MASK	0174000
#define	STEP3GOOD	RA_STEP4
		if ((sc->RAADDR->rasa & STEP3MASK) != STEP3GOOD) {
			sc->sc_state = S_IDLE;
			sc->sc_ctab.b_active = 0;
			wakeup((caddr_t)&sc->sc_ctab);
			return;
		}
		i = sc->RAADDR->rasa;
		PRINTD(("ra: Version %d model %d\n",
				i & 0xf, (i >> 4) & 0xf));
		sc->RAADDR->rasa = RA_GO;
		sc->sc_state = S_SCHAR;

		/*
		 * Initialize the data structures.
		 */
		ramsginit(sc, sc->sc_com->ra_ca.ca_rsp, sc->sc_com->ra_rsp,
			0, NRSP, RA_INT | RA_OWN);
		ramsginit(sc, sc->sc_com->ra_ca.ca_cmd, sc->sc_com->ra_cmd,
			NRSP, NCMD, RA_INT);
		bp = &sc->sc_wtab;
		bp->av_forw = bp->av_back = bp;
		sc->sc_lastcmd = 1;
		sc->sc_lastrsp = 0;
		mp = sc->sc_com->ra_cmd;
		ramsgclear(mp);
		mp->m_opcode = M_O_STCON;
		mp->m_cntflgs = M_C_ATTN | M_C_MISC | M_C_THIS;	
		((Trl *)mp->m_dscptr)->hsh |= RA_OWN|RA_INT;
		i = sc->RAADDR->raip;
		return;

	case S_SCHAR:
	case S_RUN:
		break;

	default:
		printf("ra: interrupt in unknown state %d ignored\n",
			sc->sc_state);
		return;
	}

	/*
	 * If this happens we are in BIG trouble!
	 */
	if (sc->RAADDR->rasa & RA_ERR) {
		printf("ra: fatal error (%o)\n", sc->RAADDR->rasa);
		sc->sc_state = S_IDLE;
		sc->sc_ctab.b_active = 0;
		wakeup((caddr_t)&sc->sc_ctab);
	}

	/*
	 * Check for buffer purge request
	 */
	if (sc->sc_com->ra_ca.ca_bdp) {
		PRINTD(("ra%d: buffer purge request\n", sc->sc_unit));
		sc->sc_com->ra_ca.ca_bdp = 0;
		sc->RAADDR->rasa = 0;
	}

	/*
	 * Check for response ring transition.
	 */
	if (sc->sc_com->ra_ca.ca_rspint) {
		rarspring(sc);
	}

	/*
	 * Check for command ring transition (Should never happen!)
	 */
	if (sc->sc_com->ra_ca.ca_cmdint) {
		PRINTD(("ra: command ring transition\n"));
		sc->sc_com->ra_ca.ca_cmdint = 0;
	}

	/*
	 * Waiting for command?
	 */
	if (sc->sc_cp_wait)
		wakeup((caddr_t)&sc->sc_cp_wait);

	rastart(sc);
}

/*
 * Init mscp communications area
 */
ramsginit(sc, com, msgs, offset, length, flags)
	register	ra_softcT	*sc;
	register	Trl		*com;
	register	struct mscp	*msgs;
	int				offset;
	int				length;
	int				flags;
{
	long				vaddr;

	/* 
	 * Figure out virtual address of message 
	 * skip comm area and mscp messages header and previous messages
	 */
	vaddr = (u_int)racomphys.b_un.b_addr | ((long)racomphys.b_xmem << 16);
	vaddr += (u_int)sc->sc_com - (u_int)ra_com;	/* unit offset */
	vaddr += sizeof(struct raca)			/* skip comm area */
		+sizeof(struct mscp_header);		/* m_cmdref disp */
	vaddr += offset * sizeof(struct mscp);		/* skip previous */
	while (length--) {
		com->lsh = loint(vaddr);
		com->hsh = flags | hiint(vaddr);
		msgs->m_dscptr = (long *)com;
		msgs->m_header.ra_msglen = sizeof(struct mscp);
		++com; ++msgs; vaddr += sizeof(struct mscp);
	}
}

/*
 * Try and find an unused command packet
 */
struct mscp *
ragetcp(sc)
	register ra_softcT	*sc;
{
	register struct	mscp	*mp;
	register int 		i;
	int			s;

	s = spl5();
	i = sc->sc_lastcmd;
	if ((sc->sc_com->ra_ca.ca_cmd[i].hsh & (RA_OWN|RA_INT)) == RA_INT
	    && sc->sc_credits >= 2) {
		--sc->sc_credits;
		sc->sc_com->ra_ca.ca_cmd[i].hsh &= ~RA_INT;
		mp = &sc->sc_com->ra_cmd[i];
		ramsgclear(mp);
		sc->sc_lastcmd = (i + 1) % NCMD;
		splx(s);
		return(mp);
	}

	splx(s);
	return(NULL);
}

/* Clear a mscp command packet */
ramsgclear(mp)
	register	struct	mscp	*mp;
{
	mp->m_unit = mp->m_modifier = mp->m_flags = 
		mp->m_bytecnt = mp->m_buf_l = mp->m_buf_h = 
		mp->m_elgfll = mp->m_copyspd = mp->m_elgflh =
		mp->m_opcode = mp->m_cntflgs = 0;	
}

/* Scan for response messages */
rarspring(sc)
	register	ra_softcT	*sc;
{
	register	int		i;

	sc->sc_com->ra_ca.ca_rspint = 0;
	i = sc->sc_lastrsp; 
	for (;;) {
		i %= NRSP;
		if (sc->sc_com->ra_ca.ca_rsp[i].hsh & RA_OWN)
			break;
		rarsp(&sc->sc_com->ra_rsp[i], sc);
		sc->sc_com->ra_ca.ca_rsp[i].hsh |= RA_OWN;
		++i;
	}
	sc->sc_lastrsp = i;
}

/*
 * Process a response packet
 */
rarsp(mp, sc)
	register	struct	mscp	*mp;
	register	ra_softcT	*sc;
{
	register	struct	buf 	*dp;
	struct	buf			*bp;
	ra_infoT			*disk;
	int 				st;

	/*
	 * Reset packet length and check controller credits
	 */
	mp->m_header.ra_msglen = sizeof(struct mscp);
	sc->sc_credits += mp->m_header.ra_credits & 0xf;
	if ((mp->m_header.ra_credits & 0xf0) > 0x10)
		return;

	/*
	 * If it's an error log message (datagram),
	 * pass it on for more extensive processing.
	 */
	if ((mp->m_header.ra_credits & 0xf0) == 0x10) {
		ra_error((struct mslg *)mp);
		return;
	}

	/*
	 * The controller interrupts as drive ZERO so check for it first.
	 */
	st = mp->m_status & M_S_MASK;
	if ((mp->m_opcode & 0xff) == (M_O_STCON|M_O_END)) {
		if (st == M_S_SUCC)
			sc->sc_state = S_RUN;
		else
			sc->sc_state = S_IDLE;
		sc->sc_ctab.b_active = 0;
		wakeup((caddr_t)&sc->sc_ctab);
		return;
	}

	/*
	 * Check drive and then decode response and take action.
	 */
	switch (mp->m_opcode & 0xff) {
	case M_O_ONLIN|M_O_END:
		if ((disk = sc->sc_drives[mp->m_unit]) == NULL) {
			printf("ra: couldn't ONLINE disk!\n");
			break;
		}
		dp = &disk->ra_dtab;

		if (st == M_S_SUCC) {
			/* Link the drive onto the controller queue */
			dp->b_forw = NULL;
			if (sc->sc_ctab.b_actf == NULL)
				sc->sc_ctab.b_actf = dp;
			else
				sc->sc_ctab.b_actl->b_forw = dp;
			sc->sc_ctab.b_actl = dp;

			/*  mark it online */
			radisksetup(disk, mp);
			dp->b_active = 1;
		} else {
			printf("ra%d,%d: OFFLINE\n", sc->sc_unit, mp->m_unit);
			while (bp = dp->b_actf) {
				dp->b_actf = bp->av_forw;
				bp->b_flags |= B_ERROR;
				iodone(bp);
			}
		}

		/* Wakeup in open if online came from there */
		if (mp->m_cmdref != NULL)
			wakeup((caddr_t)mp->m_cmdref);
		break;

	case M_O_AVATN:
		/* it went offline and we didn't notice */
		PRINTD(("ra%d: unit %d attention\n", sc->sc_unit, mp->m_unit));
		if ((disk = sc->sc_drives[mp->m_unit]) != NULL)
			disk->ra_dsize = 0;
		break;

	case M_O_END:
		/* controller incorrectly returns code 0200 instead of 0241 */
		PRINTD(("ra: back logical block request\n"));
		bp = (struct buf *)mp->m_cmdref;
		bp->b_flags |= B_ERROR;

	case M_O_READ | M_O_END:
	case M_O_WRITE | M_O_END:
		/* normal termination of read/write request */
		if ((disk = sc->sc_drives[mp->m_unit]) == NULL) {
			printf("ra: r/w no disk table entry\n");
			break;
		}
		bp = (struct buf *)mp->m_cmdref;

		/*
		 * Unlink buffer from I/O wait queue.
		 */
		bp->av_back->av_forw = bp->av_forw;
		bp->av_forw->av_back = bp->av_back;
		dp = &disk->ra_dtab;

#ifdef UCB_METER
		if (ra_dkn >= 0) {
			/* Messy, Should come up with a good way to do this */
			if (--dp->b_qsize == 0)
				dk_busy &= ~(1 << (ra_dkn + mp->m_unit));
		}
#endif
		if (st == M_S_OFFLN || st == M_S_AVLBL) {
			/* 
			 * mark unit offline 
			 */
			disk->ra_dsize = -1L;

			/*
			 * Link the buffer onto the front of the drive queue
			 */
			if ((bp->av_forw = dp->b_actf) == 0)
				dp->b_actl = bp;
			dp->b_actf = bp;

			/*
			 * Link the drive onto the controller queue
			 */
			if (dp->b_active == 0) {
				dp->b_forw = NULL;
				if (sc->sc_ctab.b_actf == NULL)
					sc->sc_ctab.b_actf = dp;
				else
					sc->sc_ctab.b_actl->b_forw = dp;
				sc->sc_ctab.b_actl = dp;
				dp->b_active = 1;
			}
			return;
		}
		if (st != M_S_SUCC) {
			harderr(bp, "ra");
			printf("status %o\n", mp->m_status);
			bp->b_flags |= B_ERROR;
		}
		bp->b_resid = bp->b_bcount - mp->m_bytecnt;
		iodone(bp);
		break;

	case M_O_GTUNT|M_O_END:
		break;

	default:
		printf("ra: unknown packet opcode=0%o\n", mp->m_opcode & 0xff);
		ra_error((caddr_t)mp);
	}
}


/*
 * Init disk info structure from data in mscp packet
 */
radisksetup(disk, mp)
	register	ra_infoT	*disk;
	register	struct	mscp	*mp;
{
	/* Get unit total block count */
	disk->ra_dsize = mp->m_uslow + ((long)mp->m_ushigh << 16);
	PRINTD(("ra%d: online, total size=%D, id=%d, rctsize=%d\n",
	      mp->m_unit, disk->ra_dsize, hiint(mp->m_mediaid) & 0xff,
	      mp->m_rctsize));

	/* 
	 * Get disk type and from that partition structure.  Adjust
	 * disk size to reflect revector and maint area.
	 */
	switch (hiint(mp->m_mediaid) & 0xff) {
		case 25:/* RC25 removable */
			disk->ra_size = ra25_sizes;
			disk->ra_dsize -= RC_RCT;
			break;
		case 50:/* RX50 Floppy disk */
			disk->ra_size = rd52_sizes;
			break;
		case 51:
		case 52:/* RD Hard disks */
		case 53:
			disk->ra_size = rd52_sizes;
			disk->ra_dsize -= RD_RCT;
			break;
		case 60:/* RA Hard disks */
			disk->ra_size = ra60_sizes;
			disk->ra_dsize -= RA_RCT;
			break;
		case 80:
			disk->ra_size = ra80_sizes;
			disk->ra_dsize -= RA_RCT;
			break;
		case 81:
			disk->ra_size = ra81_sizes;
			disk->ra_dsize -= RA_RCT;
			break;
		default:
			printf("ra: disk type %d unknown\n",
				hiint(mp->m_mediaid) & 0xff);
	}
}

/*
 * Process an error log message
 *
 * For now, just log the error on the console.  Only minimal decoding is done,
 * only "useful" information is printed.  Eventually should send message to
 * an error logger.
 */
ra_error(mp)
	register struct mslg *mp;
{
	printf("ra: %s error, ",
		mp->me_flags & (M_LF_SUCC|M_LF_CONT) ? "soft" : "hard");

	switch (mp->me_format) {
	case M_F_CNTERR:
		printf("controller error, event 0%o\n", mp->me_event);
		break;

	case M_F_BUSADDR:
		printf("host memory access error, event 0%o, addr 0%o\n",
			mp->me_event, mp->me_busaddr);
		break;

	case M_F_DISKTRN:
		printf("disk transfer error, unit %d, grp 0x%x, hdr 0x%x\n",
			mp->me_unit, mp->me_group, mp->me_hdr);
		break;

	case M_F_SDI:
		printf("SDI error, unit %d, event 0%o, hdr 0x%x\n",
			mp->me_unit, mp->me_event, mp->me_hdr);
		break;

	case M_F_SMLDSK:
		printf("small disk error, unit %d, event 0%o, cyl %d\n",
			mp->me_unit, mp->me_event, mp->me_sdecyl);
		break;

	default:
		printf("unknown error, unit %d, format 0%o, event 0%o\n",
			mp->me_unit, mp->me_format, mp->me_event);
	}

#ifdef RADEBUG
	/* If error debug on, do hex dump */
	{
		register char *p = (char *)mp;
		register int i;

		for (i = mp->me_header.ra_msglen; i--; /*void*/)
			printf("%x ", *p++ & 0xff);
		printf("\n");
	}
#endif
}

/*
 * RA dump routines (act like stand alone driver)
 */
#ifdef RA_DUMP

#define DBSIZE	16			/* unit of transfer, same number */

radump(dev)
	dev_t dev;
{
	register ra_softcT		*sc;
	register ra_infoT		*disk;
	register struct mscp 		*mp;
	struct mscp 			*racmd();
	daddr_t				bn, dumpsize;
	long 				paddr, maddr;
	int	 			count;
#ifdef	UNIBUS_MAP
	struct ubmap 			*ubp;
#endif
	int 				unit, partition;

	/* paranoia, space hack */
	disk = ra_disks;
	unit = RAUNIT(dev);
        sc = &ra_sc[RACON(dev)];
	partition = minor(dev) & 7;
	if (bdevsw[major(dev)].d_strategy != rastrategy || sc->RAADDR == NULL)
		return(EINVAL);

	/* Init RA controller */
	racomphys.b_un.b_addr = (caddr_t)loint(raphys((u_int)ra_com));
	racomphys.b_xmem = hiint(raphys((u_int)ra_com));
#ifdef	UNIBUS_MAP
	if (ubmap) {
		ubp = UBMAP;
		ubp->ub_lo = loint(racomphys.b_un.b_addr);
		ubp->ub_hi = hiint(racomphys.b_xmem);
		racomphys.b_un.b_addr = racomphys.b_xmem = 0;
	}
#endif

	/* Get communications area and clear out packets */
	paddr = (u_int)racomphys.b_un.b_addr
		+ ((long)racomphys.b_xmem << 16)
		+ (u_int)RINGBASE;
	sc->sc_com = ra_com;
	mp = sc->sc_com->ra_rsp;
	sc->sc_com->ra_ca.ca_cmdint = sc->sc_com->ra_ca.ca_rspint = 0;
	bzero((caddr_t)mp, 2 * sizeof(*mp));

	/* Init controller */
	sc->RAADDR->raip = 0;
	while ((sc->RAADDR->rasa & RA_STEP1) == 0)
		/*void*/;
	sc->RAADDR->rasa = RA_ERR;
	while ((sc->RAADDR->rasa & RA_STEP2) == 0)
		/*void*/;
	sc->RAADDR->rasa = loint(paddr);
	while ((sc->RAADDR->rasa & RA_STEP3) == 0)
		/*void*/;
	sc->RAADDR->rasa = hiint(paddr);
	while ((sc->RAADDR->rasa & RA_STEP4) == 0)
		/*void*/;
	sc->RAADDR->rasa = RA_GO;
	sc->sc_unit = 0;
	ramsginit(sc, sc->sc_com->ra_ca.ca_rsp, mp, 0, 2, 0);
	if (!racmd(M_O_STCON, unit, sc)) {
		PRINTB(("radump: failed to stat controller\n"));
		return(EFAULT);
	}
	PRINTB(("radump: controller up ok\n"));

	/* Bring disk for dump online */
	if (!(mp = racmd(M_O_ONLIN, unit, sc))) {
		PRINTB(("radump: failed to bring disk online\n"));
		return(EFAULT);
	}
	radisksetup(disk, mp);
 	dumpsize = MIN(disk->ra_dsize - disk->ra_size[partition].blkoff,
 			disk->ra_size[partition].nblocks);
	PRINTB(("radump: disk up ok, size=%D, type=%d\n",
		dumpsize, hiint(mp->m_mediaid) & 0xff));

	/* Check if dump ok on this disk */
	if (dumplo < 0 || dumplo >= dumpsize || dumpsize <= 0)
		return(EINVAL);
	dumpsize -= dumplo;

	/* Save core to dump partition */
#ifdef	UNIBUS_MAP
	ubp = &UBMAP[1];
#endif
	for (paddr = 0L; dumpsize > 0; dumpsize -= count) {
		count = MIN(dumpsize, DBSIZE);
		bn = dumplo + (paddr >> PGSHIFT)
			+ disk->ra_size[partition].blkoff;
		maddr = paddr;

#ifdef	UNIBUS_MAP
		/*
		 *  If UNIBUS_MAP exists, use the map.
		 */
		if (ubmap) {
			ubp->ub_lo = loint(paddr);
			ubp->ub_hi = hiint(paddr);
			maddr = (u_int)(1 << 13);
		}
#endif

		/* Write it to the disk */
		mp = &sc->sc_com->ra_rsp[1];
		mp->m_lbn_l = loint(bn);
		mp->m_lbn_h = hiint(bn);
		mp->m_bytecnt = count * NBPG;
		mp->m_buf_l = loint(maddr);
		mp->m_buf_h = hiint(maddr);
		if (racmd(M_O_WRITE, unit, sc) == 0)
			return(EIO);

		paddr += (DBSIZE << PGSHIFT);
	}

	return(0);
}

struct	mscp	*
racmd(op, unit, sc)
	int 				op, unit;
	register	ra_softcT	*sc;
{
	register	struct mscp 	*cmp, *rmp;
	Trl				*rlp;
	int 				i;

	cmp = &sc->sc_com->ra_rsp[1];
	rmp = &sc->sc_com->ra_rsp[0];
	rlp = &sc->sc_com->ra_ca.ca_rsp[0];
	cmp->m_opcode = op;
	cmp->m_unit = unit & 7;
	cmp->m_header.ra_msglen = rmp->m_header.ra_msglen = sizeof(struct mscp);
	rlp[0].hsh &= ~RA_INT;
	rlp[1].hsh &= ~RA_INT;
	rlp[0].hsh &= ~RA_INT;
	rlp[1].hsh &= ~RA_INT;
	rlp[0].hsh |= RA_OWN;
	rlp[1].hsh |= RA_OWN;
	i = sc->RAADDR->raip;
	while ((rlp[1].hsh & RA_INT) == 0)
		/*void*/;
	while ((rlp[0].hsh & RA_INT) == 0)
		/*void*/;
	sc->sc_com->ra_ca.ca_rspint = 0;
	sc->sc_com->ra_ca.ca_cmdint = 0;
	if ((rmp->m_opcode & 0xff) != (op | M_O_END)
	    || (rmp->m_status & M_S_MASK) != M_S_SUCC) {
		ra_error(unit, rmp);
		return(0);
	}
	return(rmp);
}
#endif RA_DUMP
#endif NRAC > 0 && NRAD > 0
