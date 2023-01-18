/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mbuf11.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#ifdef UCB_NET
#include "mbuf.h"

u_int	mbbase;			/* mbuffer base click address */
u_int	mbsize = (NMBUFS*MSIZE) + IOSIZE;
u_int	mbend;
u_int	miobase;		/* click of DMA area */
u_int	miosize = IOSIZE;
u_int	mbfree;			/* MBX free list */

/*
 * Get an mbuf, consisting of an inaddress header (mbuf) and an
 * external data portion (mbufx).  Try to save some time by using
 * the cache.
 */
struct mbuf *
m_bget(type)
	int type;
{
	register struct mbuf *m;
	register int s = splimp();
	register u_int click;

	if (mbicache) {
		m = mbcache[--mbicache];
		mbstat.m_mbfree--;
		mbstat.m_mtypes[type]++; mbstat.m_mtypes[MT_FREE]--;
		splx(s);
		MBTRACE(0, m);
		m->m_type = type;
		m->m_next = m->m_act = (struct mbuf *)0;
		m->m_off = MMINOFF;
		return(m);
	}

	/* otherwise do it the hard way */
	MAPSAVE();
	MSGET(m, struct mbuf, 0);
	if (m == 0)
		goto bad;
	MBXGET(click);
	if (!click) {
		MSFREE(m);
		++mbstat.m_drops;
#ifdef DIAGNOSTIC
		panic("mbuf drop");
#else
		printf("mbuf drop\n");
#endif
		goto bad;
	}
	MBX->m_mbuf = m;
	m->m_click = click;
	m->m_type = type;
	m->m_next = m->m_act = (struct mbuf *)0;
	m->m_len = 0;
	m->m_off = MMINOFF;
	mbstat.m_mbfree--;
	mbstat.m_mtypes[MT_FREE]--; mbstat.m_mtypes[type]++;
	mbstat.m_mbufs = MIN(mbstat.m_mbfree, mbstat.m_mbufs);
	goto out;
bad:
	m = 0;
out:
	MAPREST();
	splx(s);
	MBTRACE(1, m);
	return (m);
}

/*
 * Return an mbuf.  If not the last reference, just return the header;
 * else return both mbuf/mbufx.  Use the cache if empty slot exists.
 */
struct mbuf *
m_bfree(m)
	register struct mbuf *m;
{
	register struct mbuf *n;
	register u_int click;
	int s = splimp();

	MAPSAVE();
	n = m->m_next;
	MBTRACE(2, m);
#ifdef DIAGNOSTIC
	if (m->m_click < mbbase || m->m_click > mbend)
		panic("m_bfree 1");
#endif
	click = m->m_click;
	mbstat.m_mbfree++;
	mbstat.m_mtypes[MT_FREE]++;
	mbstat.m_mtypes[m->m_type]--;
	mapseg5(click, MBMAPSIZE);
	if (MBX->m_ref != 1) {
		if (MBX->m_ref < 1 || MBX->m_ref > 4)
			panic("m_bfree 2");
		MBX->m_ref--;
		MSFREE(m);
		goto out;
	}
	if (mbicache < NMBCACHE) {
		int *ip = (int *)m;
		ip--;
		if ((*ip & 01) == 0)
			panic("m_bfree 3");
		m->m_type = MT_FREE;
		m->m_next = mbcache[mbicache];
		m->m_off = 0;
		m->m_act = 0;
		mbcache[mbicache++] = m;
		goto out;
	}
	MSFREE(m);
	MBXFREE(click);
out:
	MAPREST();
	splx(s);
	return (n);
}

/*
 * Allocate a contiguous buffer for DMA IO.  Called from if_ubainit().
 * TODO: fix net device drivers to handle scatter/gather to mbufs
 * on their own; thus avoiding the copy to/from this area.
 */
memaddr
m_ioget(size)
	register int size;
{
	register u_int base;

	size = ((size + 077) & ~077);	/* round up to click boundary */
	if (size > miosize)
		return(0);
	miosize -= size;
	base = miobase;
	miobase += (size>>6);
	MBTRACE(3, base);
	return(base);
}

/*
 * C storage allocator
 * circular first-fit strategy; works with noncontiguous, but monotonically
 * linked, arena.  Each block is preceded by a ptr to the (pointer of)
 * the next following block.  Blocks are exact number of words long aligned
 * to the data type requirements of ALIGN.  Pointers to blocks must have
 * BUSY bit.  0 bit in ptr is 1 for busy, 0 for idle.  Gaps in arena are
 * merely noted as busy blocks.  Last block of arena (pointed to by alloct)
 * is empty and has a pointer to first.  Idle blocks are coalesced during
 * space search
 *
 * A different implementation may need to redefine ALIGN, NALIGN, BLOCK,
 * BUSY, INT, where INT is integer type to which a pointer can be cast.
 */
#define	INT		int
#define	ALIGN		int
#define	NALIGN		1
#define	WORD		sizeof(union store)
#define	BLOCK		1024			/* a multiple of WORD*/
#define	BUSY		1
#define	testbusy(p)	((INT)(p) & BUSY)
#define	setbusy(p)	(union store *)((INT)(p) | BUSY)
#define	clearbusy(p)	(union store *)((INT)(p) &~ BUSY)

union store {
	union store *ptr;
	ALIGN dummy[NALIGN];
	int calloc;		/* calloc clears an array of integers */
};
union store	allocs[NWORDS];	/* initial arena */
union store	*allocp;	/* search ptr */
union store	*alloct;	/* arena top */

#ifdef DIAGNOSTIC
#define	ASSERT(p)	if (!(p)) botch("p"); else
botch(s)
	char *s;
{
	printf("botch %s\n", s);
	panic("mbuf11");
}

allock()
{
	register union store *p;
	register int x;

	x = 0;
	for (p = &allocs[0]; clearbusy(p->ptr) > p; p = clearbusy(p->ptr)) {
		if (p == allocp)
			x++;
	}
	ASSERT(p == alloct);
	return(x == 1 || p == allocp);
}
#else
#define	ASSERT(p)
#endif /* DIAGNOSTIC */

char *
m_sget(nbytes, clr)
	int nbytes, clr;
{
	register union store *p, *q;
	register int nw;
	int temp, val;
	int s = splimp();

#define ret(v) { val = (v); goto out; }

	nw = (nbytes + WORD + WORD - 1) / WORD;
	ASSERT(allocp >= allocs && allocp <= alloct);
	ASSERT(allock());
	for (p = allocp;;) {
		for (temp = 0;;) {
			if (!testbusy(p->ptr)) {
				while (!testbusy((q = p->ptr)->ptr)) {
					ASSERT(q > p && q < alloct);
					p->ptr = q->ptr;
				}
				if (q >= p + nw && p + nw >= p)
					goto found;
			}
			q = p;
			p = clearbusy(p->ptr);
			if (p > q)
				ASSERT(p <= alloct);
			else if (q != alloct || p != allocs) {
				ASSERT(q == alloct && p == allocs);
				ret(NULL);
			}
			else if (++temp > 1)
				break;
		}
		/* malloc would call sbrk here and try again */
		++mbstat.m_drops;
#ifdef DIAGNOSTIC
		panic("mb drop");
#endif
		ret(NULL);
	}
found:
	allocp = p + nw;
	ASSERT(allocp <= alloct);
	if (q > allocp)
		allocp->ptr = p->ptr;
	p->ptr = setbusy(allocp);
	mbstat.m_clfree -= /* (clearbusy(p->ptr) - p) */ nw * WORD;
	mbstat.m_clusters = MIN(mbstat.m_clusters, mbstat.m_clfree);
	val = (int)(p + 1);
out:
	splx(s);
	if (val && clr)
		bzero(val, ((nbytes + 1) & ~1));
	MBTRACE(4, val);
	return((char *)val);
}

/*
 * freeing strategy tuned for LIFO allocation
 */
m_sfree(ap)
	register char *ap;
{
	register union store *p = (union store *)ap;
	int s = splimp();

	MBTRACE(5, ap);
	/* ASSERT(p > clearbusy(allocs[1].ptr) && p <= alloct); */
	ASSERT(p > allocs && p <= alloct);
	ASSERT(allock());
	/* allocp = --p; leaves old blocks around longer for debugging */
	--p;
	ASSERT(testbusy(p->ptr));
	p->ptr = clearbusy(p->ptr);
	ASSERT(p->ptr > p && p->ptr <= alloct);
	mbstat.m_clfree += ((p->ptr) - p) * WORD;
	splx(s);
}

#ifdef DIAGNOSTIC
/*
 * Mbuf address to data address, with bounds checking.  Called from
 * "mtod" macro which does type cast.
 */
mtodf(m)
	register struct mbuf *m;
{
	if (m < (struct mbuf *)&allocs[0] || m > (struct mbuf *)alloct
	    || m->m_click < mbbase || m->m_click > mbend || m->m_off < MMINOFF
	    || m->m_off > MMAXOFF || m->m_len < 0 || m->m_len > MLEN)
		panic("mtodf");
	mapseg5(m->m_click, MBMAPSIZE);
	MBX->m_mbuf = m;
	return((memaddr)MBX + m->m_off);
}
#endif

/*
 * Initialize the buffer pool.  Called from netinit/main.
 */
mbinit()
{
	register int i;
	register u_int base;

	/* setup the arena */
	m_want = 0;
	mbstat.m_words = NWORDS;
	allocs[0].ptr = &allocs[NWORDS-1];
	allocs[NWORDS-1].ptr = setbusy(&allocs[0]);
	alloct = &allocs[NWORDS-1];
	allocp = &allocs[0];
	mbstat.m_clusters = mbstat.m_clfree = (NWORDS-2) * WORD;
	/* setup DMA IO area */
	mbstat.m_iobase = miobase = mbbase;
	mbstat.m_iosize = IOSIZE;
	mbbase += (IOSIZE >> 6);
	mbsize -= IOSIZE;
	/* link the mbufs */
	mbstat.m_total = mbstat.m_mbufs = mbstat.m_mbfree = NMBUFS;
	mbstat.m_totcache = NMBCACHE;
	mbstat.m_incache = 0;
	mbstat.m_mbxbase = base = mbbase;
	mbstat.m_mbxsize = mbsize;
	mbend = mbbase + (mbsize >> 6);

	for (i = 0; i < NMBTYPES; i++)
		mbstat.m_mtypes[i] = 0;
	mbstat.m_mtypes[MT_FREE] = NMBUFS;

	mbstat.m_tbindex = mbstat.m_tbcount = NMBTBUF;

	MAPSAVE();
	for(i = 0 ; i < NMBUFS; ++i) {
		mapseg5(base, MBMAPSIZE);
		bzero((caddr_t)MBX, MSIZE);
		MBX->m_ref = 1;
		MBXFREE(base);
		base += (MSIZE >> 6);
	}
	MAPREST();
}

#ifdef DIAGNOSTIC
int	enmbtrace;			/* enable mbuf traces */

mbtrace(type, arg)
	int type, arg;
{
	extern int _ovno;
	register int *ip;
	register struct m_tbuf *mt;
	register int s;

	if (!enmbtrace)
		return;
	if (mbstat.m_tbcount > 0) {
		s = spl7();
		if (++mbstat.m_tbindex >= mbstat.m_tbcount)
			mbstat.m_tbindex = 0;
		mt = &mbstat.m_tbuf[mbstat.m_tbindex];
		mt->mt_type = (type << 4) | _ovno;
		ip = &type;  ip--;  ip--;
		ip = (int *)*ip;  ip++;
		mt->mt_pc = *ip;
		mt->mt_arg = arg;
		splx(s);
	}
}
#endif /* DIAGNOSTIC */

#ifdef OTHER_MBUF_DEBUG
int	enprint;			/* enable mbuf print routine */

mbprint(m, s)
	register struct mbuf *m;
	char *s;
{
	register int col;
	register char *ba;
	int i, bc;

	if (!enprint)
		return;
	MAPSAVE();
	nprintf("MB %s\n", s);
	for (;;) {
		if (m == 0)
			break;
		ba = mtod(m, char *);
		col = 0;  bc = m->m_len;
		nprintf("m%o next%o off%o len%o click%o act%o back%o ref%o\n",
			m, m->m_next, m->m_off, m->m_len, m->m_click, m->m_act,
			MBX->m_mbuf, MBX->m_ref);
		for (; bc; bc--) {
			i = *ba++ & 0377;
			nprintf("%o ", i);
			if (++col > 31) {
				col = 0;
				nprintf("\n  ");
			}
		}
		nprintf("\n");
		m = m->m_next;
	}
	MAPREST();
}

nprintf(fmt, x1)
	char	*fmt;
	u_int	x1;
{
	prf(fmt, &x1, 4);
}

mbmark(type, arg)
	int type, arg;
{
	extern int _ovno;

	if (mbstat.m_tbcount > 0) {
		register int *ip;
		register struct m_tbuf *mt;
		register int s = spl7();

		if (++mbstat.m_tbindex >= mbstat.m_tbcount)
			mbstat.m_tbindex = 0;
		mt = &mbstat.m_tbuf[mbstat.m_tbindex];
		mt->mt_type = (type << 4) | _ovno;
		ip = &type;  ip--;
		mt->mt_pc = *ip;
		mt->mt_arg = arg;
		splx(s);
	}
}

#endif /* OTHER_MBUF_DEBUG */
#endif /* UCB_NET */
