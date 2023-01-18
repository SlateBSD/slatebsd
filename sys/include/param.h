/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)param.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

#define	BSD	210		/* 2.10 * 10, as cpp doesn't do floats */
#define BSD2_10	1

#ifdef KERNEL
#include "localopts.h"
#include "short_names.h"
#else
#include <sys/localopts.h>
#endif

/*
 * Machine type dependent parameters.
 */
#ifdef KERNEL
#include "../machine/machparam.h"
#else
#include <machine/machparam.h>
#endif

/*
 * Network constructs
 */
#ifdef UCB_NET
#include "../machine/net.h"
#else
#include <machine/net.h>
#endif

/*
 * Machine-independent constants
 */
#define	NMOUNT	5		/* number of mountable file systems */
#define	MAXUPRC	20		/* max processes per user */
#define	NOFILE	30		/* max open files per process */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	NCARGS	5120		/* # characters in exec arglist */
#define	NGROUPS	16		/* max number groups */

#define	NOGROUP	65535		/* marker for empty group set member */

/*
 * Priorities
 */
#ifdef CGL_RTP
#define	PRTP	0
#define	PSWP	5
#else
#define	PSWP	0
#endif
#define	PINOD	10
#define	PRIBIO	20
#define	PRIUBA	24
#define	PZERO	25
#define	PPIPE	26
#define	PWAIT	30
#define	PLOCK	35
#define	PSLEP	40
#define	PUSER	50

#define	NZERO	0

/*
 * Signals
 */
#ifdef KERNEL
#include "signal.h"
#else
#include <signal.h>
#endif

#define	ISSIG(p) \
	((p)->p_sig && ((p)->p_flag&STRC || \
	 ((p)->p_sig &~ ((p)->p_sigignore | (p)->p_sigmask))) && issig())

#define	NBPW	sizeof(int)	/* number of bytes in an integer */

#define	NULL	0
#define	CMASK	022		/* default mask for file creation */
#define	NODEV	(dev_t)(-1)

/* round a number of clicks up to a whole cluster */
#define	clrnd(i)	(((i) + (CLSIZE-1)) &~ ((long)(CLSIZE-1)))

/* CBLOCK is the size of a clist block, must be power of 2 */
#define	CBLOCK	32
#define	CBSIZE	(CBLOCK - sizeof(struct cblock *))	/* data chars/clist */
#define	CROUND	(CBLOCK - 1)				/* clist rounding */

#ifndef KERNEL
#include	<sys/types.h>
#else
#include	"types.h"
#endif

/*
 * File system parameters and macros.
 *
 * The file system is made out of blocks of most MAXBSIZE units.
 */
#define	MAXBSIZE	1024

/*
 * MAXPATHLEN defines the longest permissable path length
 * after expanding symbolic links. It is used to allocate
 * a temporary buffer from the buffer pool in which to do the
 * name expansion, hence should be a power of two, and must
 * be less than or equal to MAXBSIZE.
 * MAXSYMLINKS defines the maximum number of symbolic links
 * that may be expanded in a path name. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly.
 */
#define MAXPATHLEN	1024
#define MAXSYMLINKS	8

/*
 * Macros for fast min/max.
 */
#define	MIN(a,b) (((a)<(b))?(a):(b))
#define	MAX(a,b) (((a)>(b))?(a):(b))

/*
 * Macros for counting and rounding.
 */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))

/*
 * Maximum size of hostname recognized and stored in the kernel.
 */
#define MAXHOSTNAMELEN	64
