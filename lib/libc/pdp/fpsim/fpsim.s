/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)fpsim.s	1.1 (Berkeley) 6/6/87\0>
	.even
#endif LIBC_SCCS

/*
 * Floating point simpulator.  Includes the various parts of the simulator
 * source as appropriate.
 */

#ifdef NONFP
#	include "fp0.s"
#	ifdef NONSEPARATE
#		include "fp1_nonsep.s"
#	else
#		include "fp1_sep.s"
#	endif
#	include "fp2.s"
#	include "fp3.s"
#endif
