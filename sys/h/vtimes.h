/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)vtimes.h	7.1 (Berkeley) 6/4/86
 */

/*
 * Structure returned by vtimes() and in vwait().
 * In vtimes() two of these are returned, one for the process itself
 * and one for all its children.  In vwait() these are combined
 * by adding componentwise (except for maxrss, which is max'ed).
 */
struct vtimes {
	long	vm_utime;		/* user time (60'ths) */
	long	vm_stime;		/* system time (60'ths) */
	/* divide next two by utime+stime to get averages */
	u_long	vm_idsrss;		/* integral of d+s rss */
	u_long	vm_ixrss;		/* integral of text rss */
	long	vm_maxrss;		/* maximum rss */
	long	vm_majflt;		/* major page faults */
	long	vm_minflt;		/* minor page faults */
	long	vm_nswap;		/* number of swaps */
	long	vm_inblk;		/* block reads */
	long	vm_oublk;		/* block writes */
};

#ifdef KERNEL
#endif
