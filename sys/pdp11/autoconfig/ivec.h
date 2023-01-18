/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ivec.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

struct vec_s {
	int	v_func;
	int	v_br;
} ivec;

#define	IVSIZE	(sizeof ivec)

#define	NVECTOR	100		/* max. number of vectors we can set */
