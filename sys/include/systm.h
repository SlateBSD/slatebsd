/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)systm.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Random set of variables
 * used by more than one
 * routine.
 */
extern	char version[];		/* system version */

/*
 * Nblkdev is the number of entries (rows) in the block switch.
 * Used in bounds checking on major device numbers.
 */
int	nblkdev;

/*
 * Number of character switch entries.
 */
int	nchrdev;

int	mpid;			/* generic for unique process id's */
char	runin;			/* scheduling flag */
char	runout;			/* scheduling flag */
int	runrun;			/* scheduling flag */
char	curpri;			/* more scheduling */

u_int	maxmem;			/* actual max memory per process */

int	nswap;			/* size of swap space */
int	updlock;		/* lock for sync */
daddr_t	rablock;		/* block to be read ahead */
dev_t	rootdev;		/* device of the root */
dev_t	dumpdev;		/* device to take dumps on */
long	dumplo;			/* offset into dumpdev */
dev_t	swapdev;		/* swapping device */
daddr_t	swplo;			/* block number of swap space */
dev_t	pipedev;		/* pipe device */

#ifdef BSD2_10
extern	int icode[];		/* user init code */
extern	int szicode;		/* its size */
#endif

daddr_t	bmap();

ubadr_t	clstaddr;		/* UNIBUS virtual address of clists */

extern int	cputype;	/* type of cpu = 40, 44, 45, 60, or 70 */

/*
 * Structure of the system-entry table
 */
extern struct sysent
{
	int	sy_narg;		/* total number of arguments */
	int	(*sy_call)();		/* handler */
} sysent[];

int	noproc;			/* no one is running just now */
char	*panicstr;
int	boothowto;		/* reboot flags, from boot */
int	selwait;

/* casts to keep lint happy */
#ifdef lint
#define	insque(q,p)	_insque((caddr_t)q,(caddr_t)p)
#define	remque(q)	_remque((caddr_t)q)
#endif

extern	bool_t	sep_id;		/* separate I/D */
extern	char	regloc[];	/* offsets of saved user registers (trap.c) */
extern	int	bsize;		/* size of buffers */
