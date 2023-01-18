/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)sys_socket.c	7.1 (Berkeley) 6/5/86
 */

#include "param.h"
#ifdef UCB_NET
#include "../machine/psl.h"
#include "../machine/seg.h"

#include "systm.h"
#include "user.h"
#include "file.h"
#include "domain.h"
#include "protosw.h"
#include "mbuf.h"
#include "socket.h"
#include "socketvar.h"
#include "ioctl.h"
#include "stat.h"

#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>

/*ARGSUSED*/
soo_ioctl(so, cmd, data)
	register struct socket *so;
	int cmd;
	register caddr_t data;
{

	MAPSAVE();		/* should be after switch?  KB */
	switch (cmd) {

	case FIONBIO:
		if (*(int *)data)
			so->so_state |= SS_NBIO;
		else
			so->so_state &= ~SS_NBIO;
		return (0);

	case FIOASYNC:
		if (*(int *)data)
			so->so_state |= SS_ASYNC;
		else
			so->so_state &= ~SS_ASYNC;
		return (0);

	case FIONREAD:
		*(long *)data = so->so_rcv.sb_cc;
		return (0);

	case SIOCSPGRP:
		so->so_pgrp = *(int *)data;
		return (0);

	case SIOCGPGRP:
		*(int *)data = so->so_pgrp;
		return (0);

	case SIOCATMARK:
		*(int *)data = (so->so_state&SS_RCVATMARK) != 0;
		return (0);
	}
	/*
	 * Interface/routing/protocol specific ioctls:
	 * interface and routing ioctls should have a
	 * different entry since a socket's unnecessary
	 */
#define	cmdbyte(x)	(((x) >> 8) & 0xff)
	if (cmdbyte(cmd) == 'i') {
		u.u_error = ifioctl(so, cmd, data);
		MAPUNSAVE();
		return;
	}
	if (cmdbyte(cmd) == 'r') {
		u.u_error = rtioctl(cmd, data);
		MAPUNSAVE();
		return;
	}
	u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_CONTROL, 
	    (struct mbuf *)cmd, (struct mbuf *)data, (struct mbuf *)0);
	MAPREST();
	return;
}

soo_select(fp, which)
	register struct file *fp;
	int which;
{
	register struct socket *so = fp->f_socket;
	register int retval = 0;
	int s = splnet();

	switch (which) {

	case FREAD:
		if (soreadable(so)) {
			retval++;
			goto exit;
		}
		sbselqueue(&so->so_rcv);
		break;

	case FWRITE:
		if (sowriteable(so)) {
			retval++;
			goto exit;
		}
		sbselqueue(&so->so_snd);
		break;

	case 0:
		if (so->so_oobmark ||
		    (so->so_state & SS_RCVATMARK)) {
			retval++;
			goto exit;
		}
		sbselqueue(&so->so_rcv);
		break;
	}
exit:
	splx(s);
	return (retval);
}

/*ARGSUSED*/
soo_stat(so, ub)
	register struct socket *so;
	register struct stat *ub;
{

	register int	retval;

	bzero((caddr_t)ub, sizeof (*ub));
	MAPSAVE();
	retval = (*so->so_proto->pr_usrreq)(so, PRU_SENSE,
	    (struct mbuf *)ub, (struct mbuf *)0, 
	    (struct mbuf *)0);
	MAPREST();
	return(retval);
}
#endif
