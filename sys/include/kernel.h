/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kernel.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Global variables for the kernel
 */

memaddr	malloc();

/* 1.1 */
long	hostid;
char	hostname[MAXHOSTNAMELEN];
int	hostnamelen;

/* 1.2 */
struct	timeval boottime;
struct	timeval time;
struct	timezone tz;			/* XXX */
int	hz;
int	lbolt;				/* awoken once a second */
int	realitexpire();

short	avenrun[3];

#ifdef CGL_RTP
int	wantrtp;	/* set when the real-time process is runnable */
#endif
#ifdef UCB_FRCSWAP
int	idleflg;	/* if set, allow incore forks and expands */
			/* set before idle(), cleared per second by clock */
#endif
