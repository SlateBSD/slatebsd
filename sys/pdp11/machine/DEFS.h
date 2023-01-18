/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)DEFS.h	1.1 (2.10BSD Berkeley) 2/10/87
 */

#ifndef _DEFS_
#define	_DEFS_
/*
 * Machine language assist.  Uses the C preprocessor to produce suitable code
 * for various 11's.
 */ 
#include "localopts.h"
#include "assym.h"

#ifdef UCB_NET
#	define	INTSTK	1000.		/* bytes for interrupt stack */
#else
#	define	INTSTK	500.
#endif

#ifdef PROF
#	define	HIGH	06		/* See also the :splfix files */
#	define	HIPRI	0300		/* Many spl's are done in-line */
#else
#	define	HIGH	07
#	define	HIPRI	0340
#endif

#ifdef UCB_NET
#	define	NET	02
#	define	NETPRI	0100
#endif


#if PDP11 == GENERIC			/* adapt to any 11 at boot */
#	undef	NONSEPARATE		/* support sep I/D if present */
#	define	SPLHIGH		bis	$HIPRI,PS
#	define	SPL7		bis	$0340,PS
#	define	SPLLOW		bic	$HIPRI,PS
#	ifdef UCB_NET
#		define	SPLNET	bis	$NETPRI,PS
#	endif
#else
#   ifdef NONSEPARATE			/* 11/40, 34, 23, 24 */
#	define	SPLHIGH		bis	$HIPRI,PS
#	define	SPL7		bis	$0340,PS
#	define	SPLLOW		bic	$HIPRI,PS
#	ifdef UCB_NET
#		define	SPLNET	bis	$NETPRI,PS
#	endif
#	define mfpd		mfpi
#	define mtpd		mtpi
#   else !NONSEPARATE			/* 11/44, 45, 70, 73 */
#	define SPLHIGH		spl	HIGH
#	define SPL7		spl	7
#	define SPLLOW		spl	0
#	ifdef UCB_NET
#		define	SPLNET	spl	NET
#	endif
#   endif
#endif


/*
 * Non-UNIX /bin/as instructions.  Most of these instructions are CPU
 * and configuration dependent.
 */
mfpi	= 6500^tst	/ move from previous instruction space
mtpi	= 6600^tst	/ move to previous instruction space
mfpd	= 106500^tst	/ move from previous data space
mtpd	= 106600^tst	/ move to previous data space

stst	= 170300^tst	/ store floating error status registers
ldfps	= 170100^tst	/ load floating processor status register
stfps	= 170200^tst	/ store floating processor status register

spl	= 230		/ set priority level
mfps	= 106700^tst	/ move from PS
mtps	= 106400^tst	/ move to PS
mfpt	= 000007	/ move from processor (get processor model code)

halt	= 0		/ halt cpu
wait	= 1		/ wait for interrupt
iot	= 4		/ perform I/O trap sequence
reset	= 5		/ reset all unibus devices
rtt	= 6		/ return from trap



#define	CONST(s, x, v)	DEC_/**/s(x); x=v;
#define	INT(s, x, v)	.data; .even; DEC_/**/s(x); x:; v; .text;
#define	CHAR(s, x, v)	.data; DEC_/**/s(x); x:; .byte v; .text;
#define	STRING(s, x, v)	.data; DEC_/**/s(x); x:; v; .text;
#define	SPACE(s, x, n)	.bss;  .even; DEC_/**/s(x); x:; .=.+[n]; .text;

#define	DEC_GLOBAL(x)	.globl x;
#define	DEC_LOCAL(x)

/*
 * Macros for compatibility with standard library routines that we have
 * copies of ...
 */
#define	ENTRY(x)	.globl _/**/x; _/**/x:;
#define	ASENTRY(x)	.globl x; x:;

#define	P_ENTRY(x)	.globl _/**/x; _/**/x:; PROFCODE;
#define	P_ASENTRY(x)	.globl x; x:; PROFCODE;

/*
 * PROFCODE:
 * - kernel profiling not currently implemented.
 */
#define	PROFCODE	;
#endif _DEFS_
