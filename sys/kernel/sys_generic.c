/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)sys_generic.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "user.h"
#include "proc.h"
#include "inode.h"
#include "file.h"
#include "ioctl.h"
#include "conf.h"
#include "uio.h"
#include "pty.h"
#include "kernel.h"
#include "systm.h"

read()
{
	rwuio(FREAD);
}

write()
{
	rwuio(FWRITE);
}

rwuio(mode)
	int mode;
{
	register struct a {
		int fdes;
		char *cbuf;
		u_int count;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct inode *ip;

	GETF(fp,uap->fdes);
	if (!(fp->f_flag & mode)) {
		u.u_error = EBADF;
		return;
	}
	u.u_base = (caddr_t)uap->cbuf;
	u.u_segflg = UIO_USERSPACE;
	u.u_count = uap->count;
	if (setjmp(&u.u_qsave)) {
		if (u.u_count == uap->count) {
			if ((u.u_sigintr & sigmask(u.u_procp->p_cursig)) != 0)
				u.u_error = EINTR;
			else
				u.u_eosys = RESTARTSYS;
		}
	}
	else switch(fp->f_type) {
		case DTYPE_INODE:
			ip = (struct inode *)fp->f_data;
			u.u_offset = fp->f_offset;
			if ((ip->i_mode&IFMT) == IFREG)
				ILOCK(ip);
			if (mode == FREAD)
				readi(ip);
			else {
				if (fp->f_flag&FAPPEND)
					u.u_offset = fp->f_offset = ip->i_size;
				writei(ip);
			}
			if ((ip->i_mode&IFMT) == IFREG)
				IUNLOCK(ip);
			fp->f_offset += uap->count - u.u_count;
			break;
		case DTYPE_PIPE:
			if (mode == FREAD)
				readp(fp);
			else
				writep(fp);
			break;
#ifdef UCB_NET
		case DTYPE_SOCKET:
			{
			int soreceive(), sosend();
			(*(mode == FREAD ? soreceive : sosend))
			   ((struct socket *)fp->f_socket, 0, 0, 0);
			}
			break;
#endif
	}
	u.u_r.r_val1 = uap->count - u.u_count;
}

/*
 * Ioctl system call
 */
ioctl()
{
	register struct file *fp;
	struct a {
		int	fdes;
		long	cmd;
		caddr_t	cmarg;
	} *uap;
	long com;
	register u_int k_com;
	register u_int size;
	char data[IOCPARM_MASK+1];

	uap = (struct a *)u.u_ap;
	GETF(fp, uap->fdes);
	if ((fp->f_flag & (FREAD|FWRITE)) == 0) {
		u.u_error = EBADF;
		return;
	}
	com = uap->cmd;

	/* THE 2.10 KERNEL STILL THINKS THAT IOCTL COMMANDS ARE 16 BITS */
	k_com = (u_int)com;

	if (k_com == FIOCLEX) {
		u.u_pofile[uap->fdes] |= UF_EXCLOSE;
		return;
	}
	if (k_com == FIONCLEX) {
		u.u_pofile[uap->fdes] &= ~UF_EXCLOSE;
		return;
	}

	/*
	 * Interpret high order word to find
	 * amount of data to be copied to/from the
	 * user's address space.
	 */
	size = (com &~ (IOC_INOUT|IOC_VOID)) >> 16;
	if (size > sizeof (data)) {
		u.u_error = EFAULT;
		return;
	}
	if (com&IOC_IN) {
		if (size) {
			if (((u_int)uap->cmarg|size)&1)
			    u.u_error =
				vcopyin(uap->cmarg, (caddr_t)data, size);
			else
			    u.u_error =
				copyin(uap->cmarg, (caddr_t)data, size);
			if (u.u_error)
				return;
		} else
			*(caddr_t *)data = uap->cmarg;
	} else if ((com&IOC_OUT) && size)
		/*
		 * Zero the buffer on the stack so the user
		 * always gets back something deterministic.
		 */
		bzero((caddr_t)data, size);
	else if (com&IOC_VOID)
		*(caddr_t *)data = uap->cmarg;

	switch (k_com) {

	case FIONBIO:
		u.u_error = fset(fp, FNDELAY, *(int *)data);
		return;

	case FIOASYNC:
		u.u_error = fset(fp, FASYNC, *(int *)data);
		return;

	case FIOSETOWN:
		u.u_error = fsetown(fp, *(int *)data);
		return;

	case FIOGETOWN:
		u.u_error = fgetown(fp, (int *)data);
		return;
	}
	switch(fp->f_type) {
		case DTYPE_PIPE:
		case DTYPE_INODE:
			u.u_error = ino_ioctl(fp, k_com, data);
			break;
#ifdef UCB_NET
		case DTYPE_SOCKET:
			u.u_error = soo_ioctl(fp->f_socket, k_com, data);
			break;
#endif
	}
	/*
	 * Copy any data to user, size was
	 * already set and checked above.
	 */
	if (u.u_error == 0 && (com&IOC_OUT) && size)
		if (((u_int)uap->cmarg|size)&1)
			u.u_error = vcopyout(data, uap->cmarg, size);
		else
			u.u_error = copyout(data, uap->cmarg, size);
}

int	unselect();
int	nselcoll;

/*
 * Select system call.
 */
select()
{
	register struct uap  {
		int	nd;
		fd_set	*in, *ou, *ex;
		struct	timeval *tv;
	} *uap = (struct uap *)u.u_ap;
	fd_set ibits[3], obits[3];
	struct timeval atv;
	int s, ncoll, ni;
	label_t lqsave;

	bzero((caddr_t)ibits, sizeof(ibits));
	bzero((caddr_t)obits, sizeof(obits));
	if (uap->nd > NOFILE)
		uap->nd = NOFILE;	/* forgiving, if slightly wrong */
	ni = howmany(uap->nd, NFDBITS);

#define	getbits(name, x) \
	if (uap->name) { \
		u.u_error = copyin((caddr_t)uap->name, (caddr_t)&ibits[x], \
		    (unsigned)(ni * sizeof(fd_mask))); \
		if (u.u_error) \
			goto done; \
	}
	getbits(in, 0);
	getbits(ou, 1);
	getbits(ex, 2);
#undef	getbits

	if (uap->tv) {
		u.u_error = copyin((caddr_t)uap->tv, (caddr_t)&atv,
			sizeof (atv));
		if (u.u_error)
			goto done;
		if (itimerfix(&atv)) {
			u.u_error = EINVAL;
			goto done;
		}
		s = splhigh(); timevaladd(&atv, &time); splx(s);
	}
retry:
	ncoll = nselcoll;
	u.u_procp->p_flag |= SSEL;
	u.u_r.r_val1 = selscan(ibits, obits, uap->nd);
	if (u.u_error || u.u_r.r_val1)
		goto done;
	s = splhigh();
	/* this should be timercmp(&time, &atv, >=) */
	if (uap->tv && (time.tv_sec > atv.tv_sec ||
	    time.tv_sec == atv.tv_sec && time.tv_usec >= atv.tv_usec)) {
		splx(s);
		goto done;
	}
	if ((u.u_procp->p_flag & SSEL) == 0 || nselcoll != ncoll) {
		u.u_procp->p_flag &= ~SSEL;
		splx(s);
		goto retry;
	}
	u.u_procp->p_flag &= ~SSEL;
	if (uap->tv) {
		lqsave = u.u_qsave;
		if (setjmp(&u.u_qsave)) {
			untimeout(unselect, (caddr_t)u.u_procp);
			u.u_error = EINTR;
			splx(s);
			goto done;
		}
		timeout(unselect, (caddr_t)u.u_procp, hzto(&atv));
	}
	sleep((caddr_t)&selwait, PZERO+1);
	if (uap->tv) {
		u.u_qsave = lqsave;
		untimeout(unselect, (caddr_t)u.u_procp);
	}
	splx(s);
	goto retry;
done:
#define	putbits(name, x) \
	if (uap->name) { \
		int error = copyout((caddr_t)&obits[x], (caddr_t)uap->name, \
		    (unsigned)(ni * sizeof(fd_mask))); \
		if (error) \
			u.u_error = error; \
	}
	if (u.u_error == 0) {
		putbits(in, 0);
		putbits(ou, 1);
		putbits(ex, 2);
#undef putbits
	}
}

unselect(p)
	register struct proc *p;
{
	register int s = splhigh();

	switch (p->p_stat) {

	case SSLEEP:
		setrun(p);
		break;

	case SSTOP:
		unsleep(p);
		break;
	}
	splx(s);
}

selscan(ibits, obits, nfd)
	fd_set *ibits, *obits;
	int nfd;
{
	register int which, i, j;
	register fd_mask bits;
	int flag;
	struct file *fp;
	int n = 0;
	int (*selroutine)();

#ifdef UCB_NET
	int soo_select(), ino_select();
	selroutine = (fp->f_type == DTYPE_SOCKET) ? soo_select : ino_select;
#else
	int ino_select();
	selroutine = ino_select;
#endif
	for (which = 0; which < 3; which++) {
		switch (which) {

		case 0:
			flag = FREAD; break;

		case 1:
			flag = FWRITE; break;

		case 2:
			flag = 0; break;
		}
		for (i = 0; i < nfd; i += NFDBITS) {
			bits = ibits[which].fds_bits[i/NFDBITS];
			while ((j = ffs(bits)) && i + --j < nfd) {
				bits &= ~(1L << j);
				fp = u.u_ofile[i + j];
				if (fp == NULL) {
					u.u_error = EBADF;
					break;
				}
				if ((*selroutine)(fp, flag)) {
					FD_SET(i + j, &obits[which]);
					n++;
				}
			}
		}
	}
	return (n);
}

/*ARGSUSED*/
seltrue(dev, flag)
	dev_t dev;
	int flag;
{

	return (1);
}

selwakeup(p, coll)
	register struct proc *p;
	long coll;
{
	mapinfo map;

	savemap(map);
	if (coll) {
		nselcoll++;
		wakeup((caddr_t)&selwait);
	}
	if (p) {
		register int s = splhigh();
		if (p->p_wchan == (caddr_t)&selwait) {
			if (p->p_stat == SSLEEP)
				setrun(p);
			else
				unsleep(p);
		} else if (p->p_flag & SSEL)
			p->p_flag &= ~SSEL;
		splx(s);
	}
	restormap(map);
}
