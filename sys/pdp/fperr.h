/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)fperr.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Structure of the floating point error register save/return
 */
struct fperr {
	short	f_fec;
	caddr_t	f_fea;
};
