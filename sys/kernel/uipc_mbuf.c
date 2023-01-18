/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)uipc_mbuf.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"

#ifdef UCB_NET
#include "user.h"
#include "proc.h"
#include "mbuf.h"
#include "../machine/seg.h"
#include "../netinet/in_systm.h"

/*
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */
struct mbuf *
m_get(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	return (m);
}

struct mbuf *
m_getclr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	if (m == 0)
		return (0);
	MAPSAVE();
	bzero(mtod(m, caddr_t), MLEN);
	MAPREST();
	return (m);
}

struct mbuf *
m_free(m)
	struct mbuf *m;
{
	register struct mbuf *n;

	MFREE(m, n);
	return (n);
}

/*
 * Wakeup processes waiting for mbufs.
 */
m_wakeup()
{
	mapinfo map;

	m_want = 0;
	savemap(map);
	wakeup((caddr_t)&m_want);
	restormap(map);
}

/*
 * Get more mbufs; called from MGET macro if mfree list is empty.
 * Must be called at splimp.
 */
/*ARGSUSED*/
struct mbuf *
m_more(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;
	mapinfo map;

	savemap(map);
	while ((m = m_bget(type)) == 0) {
		if (canwait == M_WAIT) {
			++m_want;
			sleep((caddr_t)&m_want, PZERO - 1);
		} else {
			mbstat.m_drops++;
			restormap(map);
			return (NULL);
		}
	}
	restormap(map);
	return (m);
}

m_freem(m)
	register struct mbuf *m;
{
	register struct mbuf *n;

	if (m == NULL)
		return;
	do {
		MFREE(m, n);
	} while (m = n);
}

/*
 * Mbuffer utility routines.
 */
struct mbuf *
m_copy(m, off, len)
	register struct mbuf *m;
	int off;
	register int len;
{
	register struct mbuf *n, **np;
	struct mbuf *top, *p;
	segm save5;

	if (len == 0)
		return (0);
	if (off < 0 || len < 0)
		panic("m_copy: < 0");
	while (off > 0) {
		if (m == 0)
			panic("m_copy: == 0");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	saveseg5(save5);
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL)
				panic("m_copy: len");
			break;
		}
		MGET(n, M_WAIT, MT_DATA);
		*np = n;
		if (n == 0)
			goto nospace;
		n->m_len = MIN(len, m->m_len - off);
#ifndef BSD2_10
		if (m->m_off > MMAXOFF) {
			p = mtod(m, struct mbuf *);
			n->m_off = ((int)p - (int)n) + off;
			mclrefcnt[mtocl(p)]++;
		} else
#endif
		{
			n->m_off = MMINOFF;
			MBCOPY(m,off,n,0,(unsigned)n->m_len);
		}
#ifdef never
		MSGET(n, struct mbuf, 0);
		*np = n;
		if (n == 0)
			goto nospace;
		n->m_len = MIN(len, m->m_len - off);
		n->m_off = m->m_off + off;
		n->m_act = n->m_next = 0;
		n->m_click = m->m_click;
		mapseg5(n->m_click,MBMAPSIZE);
		MBX->m_ref++;  
#endif never
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	goto out;
nospace:
	m_freem(top);
	top = 0;
out:
	restorseg5(save5);
	return (top);
}

m_cat(m, n)
	register struct mbuf *m, *n;
{
	while (m->m_next)
		m = m->m_next;
	while (n) {
		if (m->m_off >= MMAXOFF ||
		    m->m_off + m->m_len + n->m_len > MMAXOFF) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		MBCOPY(n, 0, m, m->m_len, (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

m_adj(mp, len)
	struct mbuf *mp;
	register int len;
{
	register struct mbuf *m, *n;

	if ((m = mp) == NULL)
		return;
	if (len >= 0) {
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_off += len;
				break;
			}
		}
	} else {
		/* a 2 pass algorithm might be better */
		len = -len;
		while (len > 0 && m->m_len != 0) {
			while (m != NULL && m->m_len != 0) {
				n = m;
				m = m->m_next;
			}
			if (n->m_len <= len) {
				len -= n->m_len;
				n->m_len = 0;
				m = mp;
			} else {
				n->m_len -= len;
				break;
			}
		}
	}
}

struct mbuf *
m_pullup(m0, len)
	struct mbuf *m0;
	int len;
{
	register struct mbuf *m, *n;
	int count;

	n = m0;
	if (len > MLEN)
		goto bad;
	MGET(m, M_DONTWAIT, MT_DATA);
	if (m == 0)
		goto bad;
	m->m_off = MMINOFF;
	m->m_len = 0;
	do {
		count = MIN(MLEN - m->m_len, len);
		if (count > n->m_len)
			count = n->m_len;
		MBCOPY(n, 0, m, m->m_len, (u_int)count);
		len -= count;
		m->m_len += count;
		n->m_off += count;
		n->m_len -= count;
		if (n->m_len)
			break;
		n = m_free(n);
	} while (n);
	if (len) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}
#endif UCB_NET
