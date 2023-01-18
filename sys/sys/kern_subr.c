/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_subr.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "systm.h"
#include "user.h"
#include "buf.h"
#include "uio.h"

uiomove(cp, n, rw)
	register caddr_t cp;
	register u_int n;
	enum uio_rw rw;
{
	register int error = 0;

	if (!n)
		return (0);
	switch (u.u_segflg) {

	case UIO_USERSPACE:
		if (n > 100 && cp + n < SEG6)
			error = uiofmove(cp, n, rw);
		else if ((n | (int)cp | (int)u.u_base) & 1)
			if (rw == UIO_READ)
				error = vcopyout(cp, u.u_base, n);
			else
				error = vcopyin(u.u_base, cp, n);
		else {
			if (rw == UIO_READ)
				error = copyout(cp, u.u_base, n);
			else
				error = copyin(u.u_base, cp, n);
		}
		if (error)
			return (error);
		break;

	case UIO_USERISPACE:
		if (n > 100 && cp + n < SEG6)
			error = uiofmove(cp, n, rw);
		else if (rw == UIO_READ)
			error = copyiout(cp, u.u_base, n);
		else
			error = copyiin(u.u_base, cp, n);
		if (error)
			return (error);
		break;

	case UIO_SYSSPACE:
		if (rw == UIO_READ)
			bcopy((caddr_t)cp, u.u_base, n);
		else
			bcopy(u.u_base, (caddr_t)cp, n);
		break;
	}
	u.u_base += n;
	u.u_count -= n;
	u.u_offset += n;
	return (error);
}

/*
 * Give next character to user as result of read.
 */
ureadc(c)
	register int c;
{
	switch (u.u_segflg) {

	case UIO_USERSPACE:
		if (subyte(u.u_base, c) < 0)
			return (EFAULT);
		break;

	case UIO_SYSSPACE:
		*u.u_base = c;
		break;

	case UIO_USERISPACE:
		if (suibyte(u.u_base, c) < 0)
			return (EFAULT);
		break;
	}
	u.u_base++;
	u.u_count--;
	u.u_offset++;
	return (0);
}

/*
 * Get next character written in by user from uio.
 */
uwritec()
{
	register int c;

	if (!u.u_count)
		return (-1);
	switch (u.u_segflg) {

	case UIO_USERSPACE:
		c = fubyte(u.u_base);
		break;

	case UIO_SYSSPACE:
		c = *u.u_base & 0377;
		break;

	case UIO_USERISPACE:
		c = fuibyte(u.u_base);
		break;
	}
	if (c < 0)
		return (-1);
	u.u_base++;
	u.u_count--;
	u.u_offset++;
	return (c & 0377);
}

/*
 * Copy bytes to/from the kernel and the user.  Uiofmove assumes the kernel
 * area being copied to or from does not overlap segment 6 - the assembly
 * language helper routine, fmove, uses segment register 6 to map in the
 * user's memory.
 */
uiofmove(cp, n, rw)
	caddr_t cp;
	register int n;
	enum uio_rw rw;
{
	register short c;
	short on;
	register short segr;		/* seg register (0 - 7) */
	u_short *segd;			/* PDR map array */
	u_short *sega;			/* PAR map array */
	int error;

#ifdef NONSEPARATE
	segd = UISD;
	sega = UISA;
#else
	if (u.u_segflg == UIO_USERSPACE && u.u_sep) {
		segd = UDSD;
		sega = UDSA;
	}
	else {
		segd = UISD;
		sega = UISA;
	}
#endif

	segr = (short)u.u_base >> 13 & 07;
	on = (short)u.u_base & 017777;
	c = MIN(n, 8192-on);
	for (;;) {
		if (rw == UIO_READ)
			error = fmove(sega[segr], segd[segr], cp, SEG6+on, c);
		else
			error = fmove(sega[segr], segd[segr], SEG6+on, cp, c);
		if (error)
			return(error);
		n -= c;
		if (!n)
			return(0);
		cp += c;
		segr++;
		segr &= 07;
		on = 0;
		c = MIN(n, 8192);
	}
	/*NOTREACHED*/
}
