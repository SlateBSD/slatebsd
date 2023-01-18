/*
 * Copyright (c) 1980, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)netisr.h	7.2 (Berkeley) 10/28/86
 */

/*
 * The networking code runs off software interrupts.
 *
 * You can switch into the network by doing splnet() and return by splx().
 * The software interrupt level for the network is higher than the software
 * level for the clock (so you can enter the network in routines called
 * at timeout time).
 */
#if defined(vax) || defined(tahoe)
#define	setsoftnet()	mtpr(SIRR, 12)
#endif

/*
 * The 11/{44,45,50,53,55,70,73,84} all have a PIRQ (Programmed interrupt
 * request register), which you can use to schedule network work, much
 * like the VAX does.  The hooks are currently in place, see scb.s and
 * mch_trap.[sh].  As far as I know, they're no faster than what we
 * currently do; they aren't a good idea because they don't work the same
 * on all machines, the 11/44 is the notable exception in that a reset
 * doesn't clear the PIRQ.  Not sure if that was the reason, but I never
 * did get certain test programs (that ran on an 11/70) to run on an 11/44.
 */
#ifdef USE_PIRQS
#define	PIREQ	((int *)0177772)
#define	setsoftnet()	(*PIREQ |= 0x0200)
#define	clearsoftnet()
#define	NETISR_CLOCK	15			/* avoids net numbers below */
#else
#define	setsoftnet()
#endif

/*
 * Each ``pup-level-1'' input queue has a bit in a ``netisr'' status
 * word which is used to de-multiplex a single software
 * interrupt used for scheduling the network code to calls
 * on the lowest level routine of each protocol.
 */
#define	NETISR_RAW	0		/* same as AF_UNSPEC */
#define	NETISR_IP	2		/* same as AF_INET */
#define	NETISR_IMP	3		/* same as AF_IMPLINK */
#define	NETISR_NS	6		/* same as AF_NS */

#define	schednetisr(anisr)	{ netisr |= 1<<(anisr); setsoftnet(); }

#ifdef KERNEL
int	netisr;				/* scheduling bits for network */
#endif
