/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)param.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/time.h"
#include "../h/resource.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/fs.h"
#include "../h/mount.h"
#include "../h/callout.h"
#include "../h/map.h"
#include "../h/clist.h"
#include "../machine/seg.h"

/*
 * System parameter formulae.
 *
 * This file is copied into each directory where we compile
 * the kernel; it should be modified there to suit local taste
 * if necessary.
 *
 */

#define	MAXUSERS %MAXUSERS%

int	hz = LINEHZ;
struct	timezone tz = { %TIMEZONE%, %DST% };
#define	NPROC (10 + 7 * MAXUSERS)
int	nproc = NPROC;
#define NTEXT (36 + MAXUSERS)
int	ntext = NTEXT;
#define NINODE ((NPROC + 16 + MAXUSERS) + 32)
int	ninode = NINODE;
#define NFILE ((8 * NINODE / 10) + 5)
int	nfile = NFILE;
#define NCALL (16 + MAXUSERS)
int	ncallout = NCALL;
int	bsize = MAXBSIZE;
int	nbuf = NBUF;

#define NCLIST (20 + 8 * MAXUSERS)
#if NCLIST > (8192 / 32)		/* 8K / sizeof(struct cblock) */
#undef NCLIST
#define NCLIST (8192 / 32)
#endif
int	nclist = NCLIST;

/*
 * These have to be allocated somewhere; allocating
 * them here forces loader errors if this file is omitted
 * (if they've been externed everywhere else; hah!).
 */
struct	proc *procNPROC;
struct	text *textNTEXT;
#ifdef UCB_METER
char	textcounted[NTEXT];		/* text performance stats */
#endif
struct	inode inode[NINODE], *inodeNINODE;
struct	file *fileNFILE;
struct	callout callout[NCALL];
struct	mount mount[NMOUNT];
struct	buf buf[NBUF], bfreelist[BQUEUES];
struct	bufhd bufhash[BUFHSZ];

#ifdef UCB_CLIST
	u_int clstdesc = ((((btoc(NCLIST*sizeof(struct cblock)))-1) << 8) | RW);
	int ucb_clist = 1;
#else
	struct cblock	cfree[NCLIST];
	int ucb_clist;
#endif

#ifdef NOKA5
	int noka5 = 1;
#else
	int noka5;
#endif

#ifdef UNIBUS_MAP
#define CMAPSIZ	(NPROC+(8*NTEXT/10))	/* size of core allocation map */
#define SMAPSIZ	(NPROC+(8*NTEXT/10))	/* size of swap allocation map */
#else
#define CMAPSIZ	NPROC
#define SMAPSIZ	(NPROC+(5*NTEXT/10))
#endif

struct mapent	_coremap[CMAPSIZ];
struct map	coremap[1] = {
	_coremap,
	&_coremap[CMAPSIZ],
	"coremap",
};

struct mapent	_swapmap[SMAPSIZ];
struct map	swapmap[1] = {
	_swapmap,
	&_swapmap[SMAPSIZ],
	"swapmap",
};

/*
 * Declarations of structures loaded last and allowed to reside in the
 * 0120000-140000 range (where buffers and clists are mapped).  These
 * structures must be extern everywhere else, and the asm output of cc
 * is edited to move these structures from comm to bss (which is last)
 * (see the script :comm-to-bss).  They are in capital letters so that
 * the edit script doesn't find some other occurrence.
 */
struct proc	PROC[NPROC];
struct file	FILE[NFILE];
struct text	TEXT[NTEXT];
