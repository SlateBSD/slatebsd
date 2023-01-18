/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)machparam.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Machine dependent constants for PDP.
 */

#define	MACHINE	"pdp"

#ifndef ENDIAN
/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */
#define	LITTLE	1234		/* least-significant byte first (vax) */
#define	BIG	4321		/* most-significant byte first */
#define	PDP	3412		/* LSB first in word, MSW first in long (pdp) */
#define	ENDIAN	PDP		/* byte order on pdp */

/*
 * Macros for network/external number representation conversion.
 */
#if ENDIAN == BIG && !defined(lint)
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)
#else
unsigned short	ntohs(), htons();
long		ntohl(), htonl();
#endif

#define	NBPG	512		/* bytes/page */
#define	PGOFSET	(NBPG-1)	/* byte offset into page */
#define	PGSHIFT	9		/* LOG2(NBPG) */

#define	DEV_BSIZE	1024
#define	DEV_BSHIFT	10		/* log2(DEV_BSIZE) */
#define	DEV_BMASK	0x3ffL		/* DEV_BSIZE - 1 */

#define	CLSIZE		2

#define	SSIZE	20		/* initial stack size (*64 bytes) */
#define	SINCR	20		/* increment of stack (*64 bytes) */

#define	USIZE	48		/* size of u-area (*64) */

/*
 * Some macros for units conversion
 */
/* Core clicks (64 bytes) to segments and vice versa */
#define	ctos(x)	(((x)+127)/128)
#define	stoc(x)	((x)*128)

/* Core clicks (64 bytes) to disk blocks */
#define	ctod(x)	(((x)+7)>>3)

/* clicks to bytes */
#define	ctob(x)	((x)<<6)

/* bytes to clicks */
#define	btoc(x)	((((unsigned)(x)+63)>>6))

/*
 * These should be fixed to use unsigned longs, if we ever get them.
 */
#define	btodb(bytes)		/* calculates (bytes / DEV_BSIZE) */ \
	((long)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)		/* calculates (db * DEV_BSIZE) */ \
	((long)(db) << DEV_BSHIFT)

/* clicks to KB */
#define	ctok(x)	(((x)>>4)&07777)

/*
 * Macros to decode processor status word.
 */
#define	USERMODE(ps)	(((ps) & PSL_USERSET) == PSL_USERSET)
#define	BASEPRI(ps)	(((ps) & PSL_IPL) == 0)

/*
 * Very crude attempt to define the relative DELAY loop speeds of various
 * processors.
 */
#ifdef PDP==44 || PDP==53 || PDP==70 || PDP==73 || PDP==83 || PDP==84
#define	DELAY(n)	{ register long N = (n)<<1; while (--N > 0); }
#else
#define	DELAY(n)	{ register long N = (n); while (--N > 0); }
#endif

/*
 * Treat ps as byte, to allow restoring value from mfps/movb
 * (see :splfix.*)
 */
#define	PS_LOBYTE	((char *)0177776)
#define	splx(ops)	(*PS_LOBYTE = ((char)(ops)))
#define	SPLX(ops) { \
	if (BASEPRI(ops) && netisr) \
		netintr(); \
	splx(ops); \
}

/*
 * high int of a long
 * low int of a long
 */
#define	hiint(long)	((int)((long)>>16))
#define	loint(long)	((int)(long))
#endif ENDIAN
