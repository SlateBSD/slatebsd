/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)uipc_syscalls.c	7.1 (Berkeley) 6/5/86
 */

#include "param.h"

#ifdef UCB_NET
#include "../machine/seg.h"
#include "../machine/psl.h"

#include "systm.h"
#include "user.h"
#include "proc.h"
#include "file.h"
#include "inode.h"
#include "buf.h"
#include "mbuf.h"
#include "protosw.h"
#include "socket.h"
#include "socketvar.h"
#include "uio.h"
#include "domain.h"
#include "vaxif/if_uba.h"
#include "netinet/in.h"
#include "netinet/in_systm.h"

/*
 * System call interface to the socket abstraction.
 */

struct file *gtsockf();

socket()
{
	register struct a {
		int	domain;
		int	type;
		int	protocol;
	} *uap = (struct a *)u.u_ap;
	struct socket *so;
	register struct file *fp;

	if ((fp = falloc()) == NULL)
		return;
	fp->f_flag = FREAD|FWRITE;
	fp->f_type = DTYPE_SOCKET;
	u.u_error = socreate(uap->domain, &so, uap->type, uap->protocol);
	if (u.u_error)
		goto bad;
	fp->f_data = (caddr_t)so;
	return;
bad:
	u.u_ofile[u.u_r.r_val1] = 0;
	fp->f_count = 0;
}

bind()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct mbuf *nam;
	char sabuf[MSIZE];

	fp = gtsockf(uap->s);
	if (fp == 0)
		return;
	nam = (struct mbuf *)sabuf;
	if (uap->namelen > MLEN)
		return (EINVAL);
	u.u_error = copyin(uap->name, MTOD(nam, caddr_t), uap->namelen);
	if (u.u_error)
		return;
	MBZAP(nam, uap->namelen, MT_SONAME);
	u.u_error = sobind((struct socket *)fp->f_data, nam);
}

listen()
{
	register struct a {
		int	s;
		int	backlog;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;

	fp = gtsockf(uap->s);
	if (fp == 0)
		return;
	u.u_error = solisten((struct socket *)fp->f_data, uap->backlog);
}

accept()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	*anamelen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct mbuf *nam;
	int namelen;
	int s;
	register struct socket *so;
	char sabuf[MSIZE];

	if (uap->name == 0)
		goto noname;
	u.u_error = copyin((caddr_t)uap->anamelen, (caddr_t)&namelen,
		sizeof (namelen));
	if (u.u_error)
		return;
#ifndef BSD2_10
	if (useracc((caddr_t)uap->name, (u_int)namelen, B_WRITE) == 0) {
		u.u_error = EFAULT;
		return;
	}
#endif
noname:
	fp = gtsockf(uap->s);
	if (fp == 0)
		return;
	s = splnet();
	so = (struct socket *)fp->f_data;
	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		u.u_error = EINVAL;
		splx(s);
		return;
	}
	if ((so->so_state & SS_NBIO) && so->so_qlen == 0) {
		u.u_error = EWOULDBLOCK;
		splx(s);
		return;
	}
	while (so->so_qlen == 0 && so->so_error == 0) {
		if (so->so_state & SS_CANTRCVMORE) {
			so->so_error = ECONNABORTED;
			break;
		}
		sleep((caddr_t)&so->so_timeo, PZERO+1);
	}
	if (so->so_error) {
		u.u_error = so->so_error;
		so->so_error = 0;
		splx(s);
		return;
	}
	if (ufalloc(0) < 0) {
		splx(s);
		return;
	}
	fp = falloc();
	if (fp == 0) {
		u.u_ofile[u.u_r.r_val1] = 0;
		splx(s);
		return;
	}
	{ struct socket *aso = so->so_q;
	  if (soqremque(aso, 1) == 0)
		panic("accept");
	  so = aso;
	}
	fp->f_type = DTYPE_SOCKET;
	fp->f_flag = FREAD|FWRITE;
	fp->f_data = (caddr_t)so;
	nam = (struct mbuf *)sabuf;
	MBZAP(nam, 0, MT_SONAME);
	u.u_error = soaccept(so, nam);
	if (uap->name) {
		if (namelen > nam->m_len)
			namelen = nam->m_len;
		/* SHOULD COPY OUT A CHAIN HERE */
		(void) copyout(MTOD(nam, caddr_t), (caddr_t)uap->name,
		    (u_int)namelen);
		(void) copyout((caddr_t)&namelen, (caddr_t)uap->anamelen,
		    sizeof (*uap->anamelen));
	}
	splx(s);
}

connect()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *nam;
	int s;
	char sabuf[MSIZE];

	fp = gtsockf(uap->s);
	if (fp == 0)
		return;
	so = (struct socket *)fp->f_data;
	if ((so->so_state & SS_NBIO) &&
	    (so->so_state & SS_ISCONNECTING)) {
		u.u_error = EALREADY;
		return;
	}
	if (uap->namelen > MLEN)
		return (EINVAL);
	nam = (struct mbuf *)sabuf;
	u.u_error = copyin(uap->name, MTOD(nam, caddr_t), uap->namelen);
	if (u.u_error)
		return;
	MBZAP(nam, uap->namelen, MT_SONAME);
	u.u_error = soconnect(so, nam);
	if (u.u_error)
		goto bad;
	if ((so->so_state & SS_NBIO) &&
	    (so->so_state & SS_ISCONNECTING)) {
		u.u_error = EINPROGRESS;
		return;
	}
	s = splnet();
	if (setjmp(&u.u_qsave)) {
		if (u.u_error == 0)
			u.u_error = EINTR;
		goto bad2;
	}
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0)
		sleep((caddr_t)&so->so_timeo, PZERO+1);
	u.u_error = so->so_error;
	so->so_error = 0;
bad2:
	splx(s);
bad:
	so->so_state &= ~SS_ISCONNECTING;
}

socketpair()
{
	register struct a {
		int	domain;
		int	type;
		int	protocol;
		int	*rsv;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp1, *fp2;
	struct socket *so1, *so2;
	int sv[2];

#ifndef BSD2_10
	if (useracc((caddr_t)uap->rsv, 2 * sizeof (int), B_WRITE) == 0) {
		u.u_error = EFAULT;
		return;
	}
#endif
	u.u_error = socreate(uap->domain, &so1, uap->type, uap->protocol);
	if (u.u_error)
		return;
	u.u_error = socreate(uap->domain, &so2, uap->type, uap->protocol);
	if (u.u_error)
		goto free;
	fp1 = falloc();
	if (fp1 == NULL)
		goto free2;
	sv[0] = u.u_r.r_val1;
	fp1->f_flag = FREAD|FWRITE;
	fp1->f_type = DTYPE_SOCKET;
	fp1->f_data = (caddr_t)so1;
	fp2 = falloc();
	if (fp2 == NULL)
		goto free3;
	fp2->f_flag = FREAD|FWRITE;
	fp2->f_type = DTYPE_SOCKET;
	fp2->f_data = (caddr_t)so2;
	sv[1] = u.u_r.r_val1;
	u.u_error = soconnect2(so1, so2);
	if (u.u_error)
		goto free4;
	if (uap->type == SOCK_DGRAM) {
		/*
		 * Datagram socket connection is asymmetric.
		 */
		 u.u_error = soconnect2(so2, so1);
		 if (u.u_error)
			goto free4;
	}
	u.u_r.r_val1 = 0;
	(void) copyout((caddr_t)sv, (caddr_t)uap->rsv, 2 * sizeof (int));
	return;
free4:
	fp2->f_count = 0;
	u.u_ofile[sv[1]] = 0;
free3:
	fp1->f_count = 0;
	u.u_ofile[sv[0]] = 0;
free2:
	(void)soclose(so2);
free:
	(void)soclose(so1);
}

sendto()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	to;
		int	tolen;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;

	msg.msg_name = uap->to;
	msg.msg_namelen = uap->tolen;
	u.u_base = uap->buf;
	u.u_count = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	sendit(uap->s, &msg, uap->flags);
}

send()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	u.u_base = uap->buf;
	u.u_count = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	sendit(uap->s, &msg, uap->flags);
}

sendmsg()
{
	register struct a {
		int	s;
		caddr_t	msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	u.u_error = copyin(uap->msg, (caddr_t)&msg, sizeof (msg));
	if (u.u_error)
		return;
	if ((u_int)msg.msg_iovlen > 1) {
		u.u_error = EMSGSIZE;
		return;
	}
	u.u_error =
	    copyin((caddr_t)msg.msg_iov, (caddr_t)&aiov,
		sizeof(aiov));
	if (u.u_error)
		return;
	u.u_base = aiov.iov_base;
	u.u_count = aiov.iov_len;
	sendit(uap->s, &msg, uap->flags);
}

sendit(s, mp, flags)
	int s;
	register struct msghdr *mp;
	int flags;
{
	register struct file *fp;
	register struct iovec *iov;
	register int i;
	struct mbuf *to;
	int len;
	char sabuf[MSIZE];

	fp = gtsockf(s);
	if (fp == 0)
		return;
	u.u_segflg = UIO_USERSPACE;
	u.u_offset = 0;				/* XXX */
	if (mp->msg_name) {
		to = (struct mbuf *)sabuf;
		u.u_error =
		    copyin(mp->msg_name, MTOD(to, caddr_t), mp->msg_namelen);
		if (u.u_error)
			return;
		MBZAP(to, mp->msg_namelen, MT_SONAME);
	} else
		to = 0;
	len = u.u_count;
	u.u_error =
	    sosend((struct socket *)fp->f_data, to, flags, (struct mbuf *)0);
	u.u_r.r_val1 = len - u.u_count;
}

recvfrom()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	from;
		int	*fromlenaddr;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	int len;

	u.u_error = copyin((caddr_t)uap->fromlenaddr, (caddr_t)&len,
	   sizeof (len));
	if (u.u_error)
		return;
	msg.msg_name = uap->from;
	msg.msg_namelen = len;
	u.u_base = uap->buf;
	u.u_count = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	recvit(uap->s, &msg, uap->flags, (caddr_t)uap->fromlenaddr, (caddr_t)0);
}

recv()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	u.u_base = uap->buf;
	u.u_count = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	recvit(uap->s, &msg, uap->flags, (caddr_t)0, (caddr_t)0);
}

recvmsg()
{
	register struct a {
		int	s;
		struct	msghdr *msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	u.u_error = copyin((caddr_t)uap->msg, (caddr_t)&msg, sizeof (msg));
	if (u.u_error)
		return;
	if ((u_int)msg.msg_iovlen > 1) {
		u.u_error = EMSGSIZE;
		return;
	}
	u.u_error =
	    copyin((caddr_t)msg.msg_iov, (caddr_t)&aiov,
		sizeof(aiov));
	if (u.u_error)
		return;
	recvit(uap->s, &msg, uap->flags,
	    (caddr_t)&uap->msg->msg_namelen,
	    (caddr_t)&uap->msg->msg_accrightslen);
}

recvit(s, mp, flags, namelenp, rightslenp)
	int s;
	register struct msghdr *mp;
	int flags;
	caddr_t namelenp, rightslenp;
{
	register struct file *fp;
	register struct iovec *iov;
	register int i;
	struct mbuf *from, *rights;
	int len;
	char sabuf[MSIZE];

	fp = gtsockf(s);
	if (fp == 0)
		return;
	u.u_segflg = UIO_USERSPACE;
	u.u_offset = 0;				/* XXX */
	len = u.u_count;
	from = (struct mbuf *)sabuf;
	u.u_error = soreceive((struct socket *)fp->f_data, &from,
	    flags, &rights);
	u.u_r.r_val1 = len - u.u_count;
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || from == 0)
			len = 0;
		else {
			if (len > from->m_len)
				len = from->m_len;
			(void) copyout((caddr_t)MTOD(from, caddr_t),
			    (caddr_t)mp->msg_name, (unsigned)len);
		}
		(void) copyout((caddr_t)&len, namelenp, sizeof (int));
	}
}

shutdown()
{
	register struct a {
		int	s;
		int	how;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;

	fp = gtsockf(uap->s);
	if (fp == 0)
		return;
	u.u_error = soshutdown((struct socket *)fp->f_data, uap->how);
}

setsockopt()
{
	register struct a {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	valsize;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct mbuf *m = NULL;
	char optbuf[MSIZE];

	fp = gtsockf(uap->s);
	if (fp == 0)
		return;
	if (uap->valsize > MLEN) {
		u.u_error = EINVAL;
		return;
	}
	if (uap->val) {
		m = (struct mbuf *)optbuf;
		u.u_error =
		    copyin(uap->val, MTOD(m, caddr_t), (u_int)uap->valsize);
		if (u.u_error)
			return;
		MBZAP(m, uap->valsize, MT_SOOPTS);
	}
	u.u_error =
	    sosetopt((struct socket *)fp->f_data, uap->level, uap->name, m);
}

getsockopt()
{
	register struct a {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	*avalsize;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct mbuf *m = NULL;
	int valsize;
	char optbuf[MSIZE];		/* XXX why not allocate an mbuf */

	fp = gtsockf(uap->s);
	if (fp == 0)
		return;
	if (uap->val) {
		u.u_error = copyin((caddr_t)uap->avalsize, (caddr_t)&valsize,
			sizeof (valsize));
		if (u.u_error)
			return;
	} else
		valsize = 0;
	m = (struct mbuf *)optbuf;
	u.u_error =
	    sogetopt((struct socket *)fp->f_data, uap->level, uap->name, m);
	if (u.u_error)
		return;
	if (uap->val && valsize && m != NULL) {
		if (valsize > m->m_len)
			valsize = m->m_len;
		u.u_error = copyout(MTOD(m, caddr_t), uap->val, (u_int)valsize);
		if (u.u_error)
			return;
		u.u_error = copyout((caddr_t)&valsize, (caddr_t)uap->avalsize,
		    sizeof (valsize));
	}
}

/*
 * Get socket name.
 */
getsockname()
{
	register struct a {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len;
	char sabuf[MSIZE];

	fp = gtsockf(uap->fdes);
	if (fp == 0)
		return;
	u.u_error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len));
	if (u.u_error)
		return;
	so = (struct socket *)fp->f_data;
	m = (struct mbuf *)sabuf;
	MAPSAVE();
	u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR, 0, m, 0);
	MAPREST();
	if (u.u_error)
		return;
	if (len > m->m_len)
		len = m->m_len;
	u.u_error = copyout(MTOD(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (u.u_error)
		return;
	u.u_error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len));
}

/*
 * Get name of peer for connected socket.
 */
getpeername()
{
	register struct a {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len;
	char sabuf[MSIZE];

	fp = gtsockf(uap->fdes);
	if (fp == 0)
		return;
	so = (struct socket *)fp->f_data;
	if ((so->so_state & SS_ISCONNECTED) == 0) {
		u.u_error = ENOTCONN;
		return;
	}
	m = (struct mbuf *)sabuf;
	u.u_error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len));
	if (u.u_error)
		return;
	MAPSAVE();
	u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_PEERADDR, 0, m, 0);
	MAPREST();
	if (u.u_error)
		return;
	if (len > m->m_len)
		len = m->m_len;
	u.u_error = copyout(MTOD(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (u.u_error)
		return;
	u.u_error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len));
}

#ifndef BSD2_10
sockargs(aname, name, namelen, type)
	struct mbuf **aname;
	caddr_t name;
	int namelen, type;
{
	register struct mbuf *m;
	int error;

	if (namelen > MLEN)
		return (EINVAL);
	m = m_get(M_WAIT, type);
	if (m == NULL)
		return (ENOBUFS);
	m->m_len = namelen;
	error = copyin(name, mtod(m, caddr_t), (u_int)namelen);
	if (error)
		(void) m_free(m);
	else
		*aname = m;
	return (error);
}
#endif

struct file *
gtsockf(fdes)
	int fdes;
{
	register struct file *fp;

	GETF(fp, fdes);
	if (fp == NULL)
		return (0);
	if (fp->f_type != DTYPE_SOCKET) {
		u.u_error = ENOTSOCK;
		return (0);
	}
	return (fp);
}
#endif
