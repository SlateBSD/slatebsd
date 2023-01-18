/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)if_uba.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "systm.h"
#include "domain.h"
#include "protosw.h"
#include "mbuf.h"
#include "buf.h"
#include "pdpuba/ubavar.h"

#ifdef UNIBUS_MAP
#include "map.h"
#include "uba.h"
#endif

#include "socket.h"
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#include <vaxif/if_uba.h>

/*
 * Routines supporting UNIBUS network interfaces.
 *
 * TODO:
 *	Support interfaces using only one BDP statically.
 */

if_ubainit(ifu, uban, hlen, nmr)
	register struct ifuba *ifu;
	int uban, hlen, nmr;		/* nmr in 64 byte clicks */
{
	if (ifu->ifu_r.ifrw_click)
		return(1);
	nmr = ctob(nmr);		/* convert clicks back to bytes */
	ifu->ifu_r.ifrw_click = m_ioget(nmr+hlen);
	ifu->ifu_w.ifrw_click = m_ioget(nmr+hlen);
	if (ifu->ifu_r.ifrw_click == 0 || ifu->ifu_w.ifrw_click == 0) {
		ifu->ifu_r.ifrw_click = ifu->ifu_w.ifrw_click = 0;
		return(0);
	}
#ifdef UNIBUS_MAP
	ifu->ifu_r.ifrw_info = ubmalloc(0, ifu->ifu_r.ifrw_click, nmr+hlen, 0);
	ifu->ifu_w.ifrw_info = ubmalloc(0, ifu->ifu_w.ifrw_click, nmr+hlen, 0);
#else
	ifu->ifu_r.ifrw_info = ((long)ifu->ifu_r.ifrw_click) * 64L;
	ifu->ifu_w.ifrw_info = ((long)ifu->ifu_w.ifrw_click) * 64L;
#endif
	ifu->ifu_hlen = hlen;
	return(1);
}

/*
 * Pull read data off a interface.
 * Len is length of data, with local net header stripped.
 * Off is non-zero if a trailer protocol was used, and
 * gives the offset of the trailer information.
 * We copy the trailer information and then all the normal
 * data into mbufs.
 */
struct mbuf *
if_rubaget(ifu, totlen, off0, ifp)
	register struct ifuba *ifu;
	int totlen, off0;
	struct ifnet *ifp;
{
	register caddr_t cp = (caddr_t)ifu->ifu_hlen;
	register struct mbuf *m;
	struct mbuf *top, **mp;
	int click = ifu->ifu_r.ifrw_click;
	int off = off0, len;

	top = 0;
	mp = &top;
	while (totlen > 0) {
		MGET(m, M_DONTWAIT, MT_DATA);
		if (m == 0) {
			m_freem(top);
			top = 0;
			goto out;
		}
		if (off) {
			len = totlen - off;
			cp = (caddr_t) (ifu->ifu_hlen + off);
		} else
			len = totlen;
nopage:
		m->m_off = MMINOFF;
		if (ifp) {
			/*
			 *	Leave room for ifp.
			 */
			m->m_len = MIN(MLEN - sizeof(ifp), len);
			m->m_off += sizeof(ifp);
		} else
			m->m_len = MIN(MLEN, len);
copy:
		copyv(click, cp, m->m_click, m->m_off,(u_int)m->m_len);
		cp += m->m_len;
nocopy:
		*mp = m;
		mp = &m->m_next;
		if (off) {
			/* sort of an ALGOL-W style for statement... */
			off += m->m_len;
			if (off == totlen) {
				cp = (caddr_t) ifu->ifu_hlen;
				off = 0;
				totlen = off0;
			}
		} else
			totlen -= m->m_len;
		if (ifp) {
			/*
			 *	Prepend interface pointer to first mbuf.
			 */
			m->m_len += sizeof(ifp);
			m->m_off -= sizeof(ifp);
			MAPSAVE();
			*(mtod(m, struct ifnet **)) = ifp;
			MAPREST();
			ifp = NULL;
		}
	}
out:
	return(top);
}

/*
 * Map a chain of mbufs onto a network interface
 * in preparation for an i/o operation.
 * The argument chain of mbufs includes the local network
 * header.
 */
if_wubaput(ifu, m)
	register struct ifuba *ifu;
	register struct mbuf *m;
{
	register struct mbuf *mp;
	u_short off = 0;
	u_short click = ifu->ifu_w.ifrw_click;

	while (m) {
		copyv(m->m_click, m->m_off, click, off, (u_int)m->m_len);
		off += m->m_len;
		MFREE(m, mp);
		m = mp;
	}
	return(off);
}

#ifdef UNIBUS_MAP
#define	KDSA	((u_short *)0172360)

#ifdef UCB_METER
extern struct ubmeter	ub_meter;
#endif
/*
 *	Map UNIBUS virtual memory over some address in kernel data
 *	space.  We're similar to the "mapalloc" routine used for
 *	raw I/O, but for different objects.
 */
ubadr_t uballoc(ubanum, addr, size, x)
	int ubanum;				/* NOTUSED */
	caddr_t addr;
	u_int size;
{
	register int nregs, s;
	register struct ubmap *ubp;
	ubadr_t paddr, vaddr;
	u_int click, first;
	int page, offset;

	page = (((int)addr >> 13) & 07);
	offset = ((int)addr & 017777);
	click = KDSA[page];
	paddr = (ubadr_t)click << 6;
	paddr += offset;
	if (!ubmap || !ub_inited)
		return(paddr);
#ifdef UCB_METER
	++ub_meter.ub_calls;
#endif
	nregs = (int) btoub(size);
	s = splhigh();
	while ((first = malloc(ub_map, nregs)) == NULL) {
#ifdef UCB_METER
		ub_meter.ub_fails++;
#endif
		ub_wantmr = 1;
		sleep(ub_map, PSWP+1);
	}
	splx(s);
#ifdef UCB_METER
	ub_meter.ub_pages += nregs;
#endif
	ubp = &UBMAP[first];
	vaddr = (ubadr_t)first << 13;

	while (nregs--) {
		ubp->ub_lo = loint(paddr);
		ubp->ub_hi = hiint(paddr);
		ubp++;
		paddr += (ubadr_t) UBPAGE;
	}
	return(vaddr);
}

/*
 *	Now for mapping an arbitrary piece of physical memory into
 *	UNIBUS virtual address space.
 */
ubadr_t ubmalloc(ubanum, addr, size, x)
	int ubanum;		/* NOTUSED */
	u_int addr, size;	/* pdp11 "clicks" */
{
	register struct ubmap *ubp;
	register int nregs, s;
	ubadr_t paddr, vaddr;
	u_int first;

	paddr = (ubadr_t)addr << 6;

	if (!ubmap || !ub_inited)
		return(paddr);

#ifdef UCB_METER
	ub_meter.ub_calls++;
#endif
	nregs = (int)btoub(size);
	s = splhigh();
	while ((first = malloc(ub_map, nregs)) == NULL) {
#ifdef UCB_METER
		ub_meter.ub_fails++;
#endif
		ub_wantmr = 1;
		sleep(ub_map, PSWP+1);
	}
	splx(s);
#ifdef UCB_METER
	ub_meter.ub_pages += nregs;
#endif
	ubp = &UBMAP[first];
	vaddr = (ubadr_t)first << 13;
	while (nregs--) {
		ubp->ub_lo = loint(paddr);
		ubp->ub_hi = hiint(paddr);
		ubp++;
		paddr += (ubadr_t) UBPAGE;
	}
	return(vaddr);
}
#endif /* UNIBUS_MAP */
