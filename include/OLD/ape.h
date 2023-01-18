/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ape.h	5.1 (Berkeley) 12/13/86
 */

typedef	struct mint
{	int len;	/* |len| = length in words of the MINT,
				sgn(len) = sgn of the MINT
				len = 0 <=> mint = 0 */
	short *val;	/* vector is "base 2^15" rep'n of the MINT */
} MINT, *PMINT;

#ifdef DBG
#include <stdio.h>
#define	shfree(u) { fprintf(stderr, "free %o\n", u); free((char *)u);}
#else !DBG
#define	shfree(u) free((char *)u)
#endif DBG

#define	xfree(x)	{if((x)->len!=0) {shfree((x)->val); (x)->len=0;}}
#define	afree(x)	{xfree(x);shfree(x);}
#define	LONGCARRY	0100000L
#define	CARRYBIT	0100000
#define	TOPSHORT	077777
#define	SHORT		LONGCARRY
#define	WORDLENGTH	15	/* length of the portion of the word we use */

extern	short	*xalloc();

extern	PMINT	ltom();
extern	PMINT	padd();
extern	PMINT	pdiv();
extern	PMINT	pgcd();
extern	PMINT	pmod();
extern	PMINT	pmult();
extern	PMINT	ppow();
extern	PMINT	prpow();
extern	PMINT	psdiv();
extern	PMINT	psqrt();
extern	PMINT	psub();
extern	PMINT	remsqrt();
extern	PMINT	shtom();
extern	PMINT	stom();

#ifdef vax
#define itom(n)	ltom((long)n)
#else !vax
#define itom(n)	shtom(n)
#endif vax
