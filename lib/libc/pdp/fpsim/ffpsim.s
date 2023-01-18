/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)ffpsim.s	1.1 (Berkeley) 6/6/87\0>
	.even
#endif LIBC_SCCS

bpt	= 3				/ break point trap

/*
 * Fake floating point trap.  See README for the ugly details ...  If this is
 * ever activated either we really tripped over an illegal instruction or the
 * library has become scrambled and needs to be reordered.  We issue a break
 * point trap when this happens.  No error message, just die.  Sorry, you
 * loose.
 */
#include "DEFS.h"

#ifdef NONFP
ASENTRY(fptrap)
	bpt
#endif
