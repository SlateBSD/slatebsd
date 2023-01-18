/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)raw_pup.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "npup.h"
#if NPUP > 0

#include "param.h"
#include "mbuf.h"
#include "socket.h"
#include "protosw.h"
#include "socketvar.h"
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netpup/pup.h>
#include <net/route.h>
#include <net/raw_cb.h>
#include <net/if.h>
#include <errno.h>

/*
 * Raw PUP protocol interface.
 */

/*
 * Encapsulate packet in PUP header which is supplied by the
 * user.  This is done to allow user to specify PUP identifier.
 */
rpup_output(m, so)
	register struct mbuf *m;
	struct socket *so;
{
	register struct rawcb *rp = sotorawcb(so);
	register struct pup_header *pup;
	int len, error = 0;
	struct mbuf *n;
	struct sockaddr_pup *dst;
	struct ifnet *ifp;

	/*
	 * Verify user has supplied necessary space
	 * for the header and check parameters in it.
	 */
	if ((m->m_off > MMAXOFF || m->m_len < sizeof(struct pup_header)) &&
	    (m = m_pullup(m, sizeof(struct pup_header))) == 0) {
		error = EMSGSIZE;	/* XXX */
		goto bad;
	}
	pup = mtod(m, struct pup_header *);
	if (pup->pup_type == 0) {
		error = EPERM;		/* XXX */
		goto bad;
	}
	if (pup->pup_tcontrol && (pup->pup_tcontrol & ~PUP_TRACE)) {
		error = EPERM;		/* XXX */
		goto bad;
	}
	for (len = 0, n = m; n; n = n->m_next)
		len += n->m_len;
	pup->pup_length = len;
#if vax || pdp11
	pup->pup_length = htons(pup->pup_length);
#endif
	/* assume user generates PUP checksum. */
	dst = (struct sockaddr_pup *)&rp->rcb_faddr;
	pup->pup_dport = dst->spup_addr;
	ifp = if_ifonnetof((u_long)UCHAR(pup->pup_dnet));
	if (ifp) {
		if (rp->rcb_flags & RAW_LADDR) {
			register struct sockaddr_pup *src;

			src = (struct sockaddr_pup *)&rp->rcb_laddr;
			pup->pup_sport = src->spup_addr;
		} else {
			pup->pup_snet = ifp->if_net;
			pup->pup_shost = ifp->if_host[0];
			/* socket is specified by user */
		}
		return ((*ifp->if_output)(ifp, m, (struct sockaddr *)dst));
	}
	error = ENETUNREACH;
bad:
	m_freem(m);
	return (error);
}
#endif
