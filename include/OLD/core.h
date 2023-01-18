/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)core.h	5.1 (Berkeley) 12/13/86
 */

/* machine dependent stuff for core files */
#define	TXTRNDSIZ	8192L
#define	stacktop(siz)	(0x10000L)
#define	stackbas(siz)	(0x10000L-(siz))
