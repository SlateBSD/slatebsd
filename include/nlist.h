/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)nlist.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Format of a symbol table entry; this file is included by <a.out.h>
 * and should be used if you aren't interested the a.out header
 * or relocation information.
 */
struct	nlist {
	char	n_name[8];	/* symbol name */
	int	n_type;		/* type flag, i.e. N_TEXT etc; see below */
	unsigned int n_value;	/* value */
};

/*
 * Simples values for n_type.
 */
#define	N_UNDF	0x0		/* undefined */
#define	N_ABS	0x1		/* absolute */
#define	N_TEXT	0x2		/* text */
#define	N_DATA	0x3		/* data */
#define	N_BSS	0x4		/* bss */
#define	N_REG	0x14		/* register name */
#define	N_FN	0x1f		/* file name symbol */

#define	N_EXT	0x20		/* external bit, or'ed in */
#define	N_TYPE	0x1f		/* mask for all the type bits */

/*
 * Format for namelist values.
 */
#define	N_FORMAT	"%06o"
