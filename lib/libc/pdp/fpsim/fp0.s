/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)fp0.s	1.1 (Berkeley) 6/6/87\0>
	.even
#endif LIBC_SCCS

/*
 * Always define fpsim so the reference in ../stdio/fltpr.o can be resolved no
 * matter what kind of library we're gerating.  See comments in README.
 */
.globl	fpsim
fpsim:

/*
 * Variable definitions module of floating point simulation module.
 */

.data
reenter: 1

.bss
asign:	.=.+2
areg:	.=.+8
aexp:	.=.+2
bsign:	.=.+2
breg:	.=.+8
bexp:	.=.+2

fpsr:	.=.+2
trapins: .=.+2

ac0:	.=.+8.
ac1:	.=.+8.
ac2:	.=.+8.
ac3:	.=.+8.
ac4:	.=.+8.
ac5:	.=.+8.

sr0:	.=.+2
sr1:	.=.+2
	.=.+2
	.=.+2
	.=.+2
	.=.+2
ssp:	.=.+2
spc:	.=.+2
sps:	.=.+2
pctmp:	.=.+8

#ifndef NONSEPARATE
tr0:	.=.+2
tr1:	.=.+2
#endif

.text
