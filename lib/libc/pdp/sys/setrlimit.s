/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid: <@(#)setrlimit.s	2.5 (Berkeley) 2/2/87\0>
	.even
#endif SYSLIBC_SCCS

#include "SYS.h"

SYSCALL(setrlimit,norm)
