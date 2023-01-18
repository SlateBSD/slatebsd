/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)iopage.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * I/O page addresses
 */

#define	PS		((u_short *) 0177776)
#define	SL		((u_short *) 0177774)
#define	PIR		((u_short *) 0177772)
#define	CPUERR		((u_short *) 0177766)

#if	PDP11 == 70 || PDP11 == GENERIC
#define	SYSSIZEHI	((u_short *) 0177762)
#define	SYSSIZELO	((u_short *) 0177760)
#endif

#if	PDP11 == 44 || PDP11 == 70 || PDP11 == GENERIC
#define	MEMSYSCTRL	((u_short *) 0177746)
#define	MEMSYSERR	((u_short *) 0177744)
#define	MEMERRHI	((u_short *) 0177742)
#define	MEMERRLO	((u_short *) 0177740)
#endif

/* Memory control registers (one per memory board) */
#define	MEMSYSMCR	((u_short *) 0172100)
#define	MEMMCR_EIE	0000001		/* Error Indication Enable */

#define	CSW		((u_short *) 0177570)
