/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ioconf.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "systm.h"

dev_t	rootdev = %ROOTDEV%,
	swapdev = %SWAPDEV%,
	pipedev = %PIPEDEV%;
daddr_t swplo = (daddr_t)%SWAPLO%;
int	nswap = %NSWAP%;

dev_t	dumpdev = %DUMPDEV%;
daddr_t	dumplo = (daddr_t)%DUMPLO%;
int	%DUMPROUTINE%();
int	(*dump)() = %DUMPROUTINE%;

#ifdef UCB_NET
#include "il.h"
#include "sri.h"
#include "vv.h"
#include "buf.h"
#include "qe.h"
#include "pdpuba/ubavar.h"

#if NDE > 0
extern struct uba_driver	dedriver;
#endif
#if NIL > 0
extern struct uba_driver	ildriver;
#endif
#if NQE > 0
extern struct uba_driver	qedriver;
#endif
#if NSRI > 0
extern struct uba_driver	sridriver;
#endif
#if NVV > 0
extern struct uba_driver	vvdriver;
#endif

struct uba_device ubdinit[] = {
#if NDE > 0
	{ &dedriver,	0,0, (caddr_t)0174510 },
#endif
#if NIL > 0
	{ &ildriver,	0,0, (caddr_t)0164000 },
#endif
#if NQE > 0
	{ &qedriver,	0,0, (caddr_t)0174440 },
#endif
#if NSRI > 0
	{ &sridriver,	0,0, (caddr_t)0167770 },
#endif
#if NVV > 0
	{ &vvdriver,	0,0, (caddr_t)0161000 },
#endif
	0,
};
#endif /* UCB_NET */
