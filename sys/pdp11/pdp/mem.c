/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mem.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "user.h"
#include "conf.h"
#include "uio.h"
#include "hk.h"
#include "xp.h"

mmread(dev)
	dev_t dev;
{

	return (mmrw(dev, UIO_READ));
}

mmwrite(dev)
	dev_t dev;
{

	return (mmrw(dev, UIO_WRITE));
}

/*
 * This routine is callable only from the high
 * kernel as it assumes normal mapping and doesn't
 * bother to save R5.
 */
mmrw(dev, rw)
	dev_t dev;
	enum uio_rw rw;
{
	switch (minor(dev)) {

	case 0: /* minor device 0 is physical memory */
		{
			register u_int on;
			register int error;

			while (u.u_count) {
				mapseg5((memaddr)(u.u_offset>>6),
				   ((btoc(8192)-1)<<8)|RW);
				on = u.u_offset & 077L;
				error = uiomove(SEG5+on,
				    MIN(u.u_count, 8192-on), rw);
				if (error)
					break;
			}
			normalseg5();
			return(error);
		}

	case 1: /* minor device 1 is kernel memory */
		return(uiomove((caddr_t)u.u_offset, (int)u.u_count, rw));

	case 2: /* minor device 2 is EOF/RATHOLE */
		if (rw == UIO_READ)
			return(0);
		u.u_base += u.u_count;
		u.u_offset += u.u_count;
		u.u_count = 0;
		return(0);
	}
}

#if NHK > 0 || NXPD > 0
/*
 * Internal versions of mmread(), mmwrite()
 * used by disk driver ecc routines.
 */
getmemc(addr)
	long addr;
{
	register int a, c, d;

	/*
	 * bn = addr >> 6
	 * on = addr & 077
	 */
	a = UISA[0];
	d = UISD[0];
	UISA[0] = addr >> 6;
	UISD[0] = RO;			/* one click, read only */
	c = fuibyte((caddr_t)(addr & 077));
	UISA[0] = a;
	UISD[0] = d;
	return(c);
}

putmemc(addr,contents)
	long addr;
	int contents;
{
	register int a, d;

	/*
	 * bn = addr >> 6
	 * on = addr & 077
	 */
	a = UISA[0];
	d = UISD[0];
	UISA[0] = addr >> 6;
	UISD[0] = RW;			/* one click, read/write */
	suibyte((caddr_t)(addr & 077), contents);
	UISA[0] = a;
	UISD[0] = d;
}
#endif
