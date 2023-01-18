/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)a.out.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Definitions of the a.out header
 * and magic numbers are shared with
 * the kernel.
 */
#include <sys/exec.h>

/*
 * Macros which take exec structures as arguments and tell whether
 * the file has a reasonable magic number or offset to text.
 */
#define	N_BADMAG(x) \
	(((x).a_magic)!=A_MAGIC1 && ((x).a_magic)!=A_MAGIC2 && \
	((x).a_magic)!=A_MAGIC3 && ((x).a_magic)!=A_MAGIC4 && \
	((x).a_magic)!=A_MAGIC5 && ((x).a_magic)!=A_MAGIC6)

#define	N_TXTOFF(x) \
	((x).a_magic==A_MAGIC5 || (x).a_magic==A_MAGIC6 ? \
	sizeof(struct ovlhdr) + sizeof(struct exec) : sizeof(struct exec))

/*
 * Format of a symbol table entry; this file is included by <a.out.h>
 * and should be used if you aren't interested the a.out header
 * or relocation information.
 */
struct	nlist {
	char	n_name[8];	/* symbol name */
	int	n_type;		/* type flag */
unsigned int	n_value;	/* value */
};

/*
 * Simple values for n_type.
 */
#define	N_UNDF	0x0		/* undefined */
#define	N_ABS	0x1		/* absolute */
#define	N_TEXT	0x2		/* text symbol */
#define	N_DATA	0x3		/* data symbol */
#define	N_BSS	0x4		/* bss symbol */
#define	N_REG	0x14		/* register name */
#define	N_FN	0x1f		/* file name symbol */

#define	N_EXT	0x20		/* external bit, or'ed in */
#define	N_TYPE	0x1f		/* mask for all the type bits */

/*
 * Format for namelist values.
 */
#define	N_FORMAT	"%06o"
