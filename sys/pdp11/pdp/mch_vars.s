/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mch_vars.s	1.1 (2.10BSD Berkeley) 2/10/87
 */
#include "DEFS.h"
#include "../machine/mch_iopage.h"

CONST(GLOBAL, _u, 0140000)

#ifndef NONFP
	INT(GLOBAL, fpp, 0)		/ we have a floating point processor
#endif

CHAR(GLOBAL, _ubmap, 0)			/ we have a unibus map
CHAR(GLOBAL, _sep_id, 0)		/ we have a separate I&D CPU

#ifdef ENABLE34
	CHAR(GLOBAL, _enable34, 0)	/ we have an ABLE Enable/34 network board
#endif
.even


/*
 * Define _ka6 and give it a reasonable initial value
 */
#ifdef KERN_NONSEP
#	ifdef ENABLE34
		INT(GLOBAL, _ka6, DEC_KISA6)
#	else
		INT(GLOBAL, _ka6, KISA6)
#	endif

	INT(GLOBAL, _cputype, 40.)
#else
#	ifdef ENABLE34
		INT(GLOBAL, _ka6, DEC_KDSA6)
#	else
		INT(GLOBAL, _ka6, KDSA6)
#	endif

	INT(GLOBAL, _cputype, 45.)
#endif


SPACE(GLOBAL, intstk, INTSTK)		/ temp stack while KDSA6 is unmapped
CONST(GLOBAL, eintstk, intstk+INTSTK)	/ top of interuupt stack
INT(GLOBAL, nofault, 0)			/ address of temp fault trap handler
