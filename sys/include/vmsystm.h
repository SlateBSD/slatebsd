/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)vmsystm.h	7.1 (Berkeley) 6/4/86
 */

/*
 * Miscellaneous virtual memory subsystem variables and structures.
 */

#if defined(KERNEL) && defined(UCB_METER)
size_t	freemem;		/* remaining clicks of free memory */
u_short	avefree;		/* moving average of remaining free clicks */
u_short	avefree30;		/* 30 sec (avefree is 5 sec) moving average */
int	multprog;		/* current multiprogramming degree */

/* writable copies of tunables */
int	maxslp;			/* max sleep time before very swappable */
#endif

/*
 * Fork/vfork accounting.
 */
struct	forkstat
{
	long	cntfork;
	long	cntvfork;
	long	sizfork;
	long	sizvfork;
};
#if defined(KERNEL) && defined(UCB_METER)
struct	forkstat forkstat;
#endif
