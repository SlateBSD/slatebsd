/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rdwrv.c	1.1 (Berkeley) 1/19/87";
#endif LIBC_SCCS and not lint

/*
 * Fake 4.3BSD's readv and writev system calls - simply loop through the iov
 * structure ...  Note that as with all I/O a return value of (int) < 0 may
 * mean that the amount of I/O was simply greater than the maximum positive
 * signed int ...
 */
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

readv(d, iov, iovcnt)
	int d;
	struct iovec *iov;
	u_int iovcnt;
{
	extern u_int read();
	struct sgttyb ttyb;

	if (ioctl(d, TIOCGETP, &ttyb) < 0 || ttyb.sg_flags & (RAW|CBREAK))
		return(rdwrv(read, d, iov, iovcnt));
	else {
		/*
		 * We've been given a tty in line buffered input mode to
		 * read from.  Too hard to integrate this into the general
		 * rdwrv below, so we handle it here.
		 */
		register int cc, vcc;
		register struct iovec *iop;
		char ttbuf[CANBSIZ];	/* one tty line at most */
		register char *bp;
		int nread;

		for (cc = 0, iop = iov, vcc = iovcnt; vcc; iop++, vcc--)
			cc += iop->iov_len; 
		if ((nread = read(d, ttbuf, cc)) != 0 && (int)nread != -1)
			for (bp = ttbuf, iop = iov, cc = nread; cc; iop++) {
				vcc = MIN(cc, iop->iov_len);
				bcopy(bp, iop->iov_base, vcc);
				bp += vcc;
				cc -= vcc;
			}
		return(nread);
	}
}

writev(d, iov, iovcnt)
	int d;
	struct iovec *iov;
	int iovcnt;
{
	extern u_int write();
	return(rdwrv(write, d, iov, iovcnt));
}

static
rdwrv(rdwr, d, iov, iovcnt)
	u_int (*rdwr)();
	int d;
	register struct iovec *iov;
	int iovcnt;
{
	register u_int vcc, n;
	register char *vcp;
	u_int cc;

	cc = 0;
	while (iovcnt) {
		vcc = iov->iov_len;
		vcp = iov->iov_base;
		cc += vcc;
		iovcnt--;
		iov++;
		while (vcc)
			if ((n = rdwr(d, vcp, vcc)) == 0 || (int)n == -1)
				return(n ? -1 : (int)(cc-vcc));
			else {
				vcc -= n;
				vcp += n;
			}
	}
	return(cc);
}
