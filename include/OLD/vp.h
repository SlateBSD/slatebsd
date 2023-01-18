/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)vp.h	5.1 (Berkeley) 12/13/86
 */

/* see vp(4) */
#include <sys/vcmd.h>

int	sppmode[]	= {0400, 0, 0};	/* enter spp */
int	pltmode[]	= {0200, 0, 0};	/* enter plot */
int	prtmode[]	= {0100, 0, 0};	/* enter print */
int	clrcom[]	= {0404, 0, 0};	/* remote clear, enter spp */
int	termcom[]	= {0240, 0, 0};	/* remote terminate, enter plot */
int	ffcom[]		= {0220, 0, 0};	/* plot mode form feed */
