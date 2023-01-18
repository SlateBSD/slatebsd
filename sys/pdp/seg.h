/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)seg.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

#ifndef _SEG_
#define _SEG_

/*
 * Access abilities
 */
#define RO	02		/* Read only */
#define RW	06		/* Read and write */
#define NOACC	0		/* Abort all accesses */
#define ACCESS	07		/* Mask for access field */
#define ED	010		/* Extension direction */
#define TX	020		/* Software: text segment */
#define ABS	040		/* Software: absolute address */

u_short *ka6;			/* nonseparate:  KISA6; separate:  KDSA6 */
u_int	kdsa6;			/* saved KDSA6, if any */

/*
 * Addresses and bits for DEC's KT-11
 * and macros for remapping kernel data space.
 */
#define UISD	((u_short *) 0177600)	/* first user I-space descriptor */
#define UDSD	((u_short *) 0177620)	/* first user D-space descriptor */

#ifdef	ENABLE34
#	define DEC_UISA7	((u_short *) 0177656)
	extern u_short *UISA, *UDSA, *KISA0, *KDSA1, *KDSA2, *KDSA5, *KDSA6;
#else
#	define UISA	((u_short *) 0177640)	/* first user I-space address */
#	define UDSA	((u_short *) 0177660)	/* first user D-space address */
#endif

#ifndef ENABLE34
#	define KISA0	((u_short *) 0172340)
#	define KISA1	((u_short *) 0172342)
#	define KISA5	((u_short *) 0172352)
#	define KISA6	((u_short *) 0172354)
#	define SSR3	((u_short *) 0172516)
#endif

#define	KISD1	((u_short *) 0172302)
#define	KISD5	((u_short *) 0172312)
#define	KISD6	((u_short *) 0172314)

#ifdef KERN_NONSEP
#	ifndef ENABLE34
#		define KDSA5	KISA5
#		define KDSA6	KISA6
#	endif !ENABLE34
#	define KDSD5	KISD5
#	define KDSD6	KISD6
#else
#	ifndef ENABLE34
#		define  KDSA5	((u_short *) 0172372)
#		define  KDSA6	((u_short *) 0172374)
#	endif !ENABLE34
#	define KDSD5	((u_short *) 0172332)
#	define KDSD6	((u_short *) 0172334)
#endif

/* start of possibly mapped area; see machdep.c */
#define	REMAP_AREA	((caddr_t)&proc[0])
#define	SEG5		((caddr_t) 0120000)
#define	SEG6		((caddr_t) 0140000)
#define	SEG7		((caddr_t) 0160000)

#ifdef COMMENT
 * Macros for resetting the kernel segmentation registers to map in
 * out-of-address space data.  If KDSA5 is used for kernel data space
 * only proc, file and text tables are allowed in that range.  Routines
 * can repoint KDSA5 to map in data such as buffers or clists without
 * raising processor priority by calling these macros.  Copy (in mch.s) uses
 * two registers, KDSA5 and 6. If KDSA6 is in use, the prototype register
 * kdsa6 will be non-zero, and the kernel will be running on a temporary stack
 * in bss.  Interrupt routines that access any of the structures in this range
 * or the u. must call savemap (in machdep.c) to save the current mapping
 * information in a local structure and restore it before returning.
 *
 * USAGE:
 *	To repoint KDSA5 from the top level,
 *		mapseg5(addr, desc);	/* KDSA5 now points at addr */
 *		...
 *		normalseg5();		/* normal mapping */
 *
 *	To repoint KDSA5 from interrupt or top level,
 *		segm saveregs;
 *		saveseg5(saveregs);	/* save previous mapping of segment 5 */
 *		mapseg5(addr, desc);	/* KDSA5 now points at addr */
 *		...
 *		restorseg5(saveregs);	/* restore previous mapping */
 *
 *	To access proc, text, file or user structures from interrupt level,
 *		mapinfo map;		/* save ALL mapping information, */
 *		savemap(map);		/* restore normal mapping of KA5 */
 *		...			/* and 6 */
 *		restormap(map);		/* restore previous mapping */
#endif /* COMMENT */

/*
 * Structure to hold a saved PAR/PDR pair.
 */
struct segm_reg {
	u_int	se_desc;
	u_int	se_addr;
};
typedef struct segm_reg segm;
typedef struct segm_reg mapinfo[2];	/* KA5, KA6 */

#ifndef DIAGNOSTIC
#	ifdef UCB_NET
		extern segm Bmapsave;
#		define	mapout(bp)	restorseg5(Bmapsave)
#	else
#		define	mapout(bp)	normalseg5()
#	endif
#endif

/* use segment 5 to access the given address. */
#define mapseg5(addr,desc) { \
	*KDSD5 = desc; \
	*KDSA5 = addr; \
}

/* save the previous contents of KDSA5/KDSD5. */
#define saveseg5(save) { \
	save.se_addr = *KDSA5; \
	save.se_desc = *KDSD5; \
}

/* restore normal kernel map for seg5. */
#ifdef NOKA5
#	define	normalseg5()
#else
	extern segm	seg5;		/* prototype KDSA5, KDSD5 */
#	define normalseg5()	restorseg5(seg5)
#endif

/* restore the previous contents of KDSA5/KDSD5. */
#define restorseg5(save) { \
	*KDSD5 = save.se_desc; \
	*KDSA5 = save.se_addr; \
}

caddr_t	mapin();
#endif /* _SEG_ */
