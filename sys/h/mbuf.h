/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mbuf.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Constants related to memory allocator.
 */
#define	MSIZE		256			/* size of an mbuf */

#define	MMINOFF		4			/* mbuf header length */
#define	MTAIL		0
#define	MMAXOFF		(MSIZE-MTAIL)		/* offset where data ends */
#define	MLEN		(MSIZE-MMINOFF-MTAIL)	/* mbuf data length */

#define	IOSIZE		8192			/* area for DMA buffers */
#define	NMBUFS		80			/* mbuf area = NMBUFS*MSIZE */
#define	NMBCACHE	80			/* no. of cached mbufs */
#define	NWORDS		2750	/* init arena size, bytes=(NWORDS-2)*WORD */

#define	MBX		((struct mbufx *)0120000)
#define	MBMAPSIZE	(((btoc(MSIZE)-1)<<8)|RW)	/* MSIZE clicks */

/*
 * Macros for type conversion
 */

/* address in mbuf to mbuf head */
#ifdef DIAGNOSTIC
#define	dtom(x)		(MBX->m_mbuf->m_click == *KDSA5 ? \
				MBX->m_mbuf : (struct mbuf *)panic("dtom"))
#else
#define	dtom(x)		MBX->m_mbuf
#endif

/* mbuf head, to typed data */
#define	mtod(x,t)	((t)(mtodf(x)))
#define	MTOD(x,t)	((t)(&((char *)x)[sizeof(struct mbuf)+MMINOFF]))

#define	MBZAP(m, len, type) \
	(m)->m_next = 0; (m)->m_off = MMINOFF; (m)->m_len = (len); \
	(m)->m_type = (type); (m)->m_act = 0;

struct mbuf {				/* in address space part of mbuf */
	struct	mbuf *m_next;		/* next buffer in chain */
	short	m_off;			/* offset of data */
	short	m_len;			/* amount of data in this mbuf */
	short	m_type;			/* type of buffer */
	short	m_click;		/* MMU address */
	struct	mbuf *m_act;		/* link in higher-level mbuf list */
};

struct mbufx {				/* external part of mbuf */
	struct	mbuf *m_mbuf;		/* back pointer to mbuf */
	short	m_ref;			/* reference counter */
	u_char	m_dat[MLEN];		/* data storage */
};

/* mbuf types */
#define	MT_FREE		0	/* should be on free list */
#define	MT_DATA		1	/* dynamic (data) allocation */
#define	MT_HEADER	2	/* packet header */
#define	MT_SOCKET	3	/* socket structure */
#define	MT_PCB		4	/* protocol control block */
#define	MT_RTABLE	5	/* routing tables */
#define	MT_HTABLE	6	/* IMP host tables */
#define	MT_ATABLE	7	/* address resolution tables */
#define	MT_SONAME	8	/* socket name */
#define	MT_ZOMBIE	9	/* zombie proc status */
#define	MT_SOOPTS	10	/* socket options */
#define	MT_FTABLE	11	/* fragment reassembly header */
#define	MT_RIGHTS	12	/* access rights */
#define	MT_IFADDR	13	/* interface address */
#define	NMBTYPES	16	/* no. of mbuf types (must be power of 2) */

/* flags to m_get */
#define	M_DONTWAIT	0
#define	M_WAIT		1

#define	M_NOCLEAR	0
#define	M_CLEAR		1

/* length to m_copy to copy all */
#define	M_COPYALL	077776

#define	MGET(m, i, t) \
	{ int ms = splimp(); \
	  if (mbstat.m_incache) \
		{ (m) = mbcache[--mbstat.m_incache]; \
		  if ((m)->m_type != MT_FREE) panic("mget"); (m)->m_type = t; \
		  mbstat.m_mtypes[t]++; mbstat.m_mtypes[MT_FREE]--; \
		  mbstat.m_mbfree--; MBTRACE(8, m); \
		  (m)->m_next = 0; \
		  (m)->m_off = MMINOFF; } \
	  else \
		(m) = m_more(i, t); \
	  splx(ms); }

#define	MFREE(m, n) \
	{ int ms = splimp(); \
	  if ((m)->m_type == MT_FREE) panic("mfree"); \
	  if (mbstat.m_incache < mbstat.m_totcache) { \
	    mbstat.m_mtypes[(m)->m_type]--; mbstat.m_mtypes[MT_FREE]++; \
	    mbstat.m_mbfree++; MBTRACE(9, m); \
	    (m)->m_type = MT_FREE; \
	    (n) = (m)->m_next; (m)->m_next = mbcache[mbstat.m_incache]; \
	    (m)->m_off = 0; (m)->m_act = 0; mbcache[mbstat.m_incache++] = (m); \
	  } else \
	    (n) = m_bfree((m)); \
	  splx(ms); \
	  if (m_want) \
		m_wakeup(); \
	}

#define	MSGET(sp,s,clr)	sp = (s *)m_sget(sizeof(s),clr)
#define	MSFREE(sp)	(void)m_sfree((caddr_t)sp)

#define	MBXGET(c) { \
	if ((c) = mbfree) { \
		mapseg5(mbfree,MBMAPSIZE); \
		if (MBX->m_ref) \
			panic("MBXGET"); \
		MBX->m_ref = 1; \
		MBTRACE(6,mbfree); \
		mbfree = (u_int)MBX->m_mbuf; \
	} \
}

#define	MBXFREE(c) { \
	mapseg5(c, MBMAPSIZE); \
	if (MBX->m_ref != 1) \
		panic("MBXFREE"); \
	MBX->m_ref = 0; \
	MBTRACE(7,c); \
	MBX->m_mbuf = (struct mbuf *)mbfree; \
	mbfree = (c); \
}

#define	MBCOPY(from, fromoff, to, tooff, count) \
		copyv(from->m_click, from->m_off + fromoff, \
		to->m_click, to->m_off + tooff, (u_int)count)

#define	MAPSAVE()	{ segm ZZMAP; saveseg5(ZZMAP)
#define	MAPUNSAVE()	restorseg5(ZZMAP)
#define	MAPREST()	restorseg5(ZZMAP); }

/*
 * Mbuf statistics.
 */
struct mbstat {
	short	m_mbufs;	/* mbufs obtained from page pool */
	short	m_mbfree;	/* mbufs on our free list */
	short	m_clusters;	/* clusters obtained from page pool */
	short	m_clfree;	/* free clusters */
	short	m_drops;	/* times failed to find space */
	short	m_mtypes[NMBTYPES];	/* type specific mbuf allocations */
#ifdef BSD2_10
	u_int	m_words;	/* no. of words in arena */
	u_int	m_total;	/* total no. of mbufs */
	u_int	m_incache;	/* no. of entries in cache */
	u_int	m_totcache;	/* total no. of cached mbufs */
	u_int	m_iobase;	/* click for dma i/o bufs */
	u_int	m_iosize;	/* no. of bytes for dma i/o mbufs */
	u_int	m_mbxbase;	/* click for mbufx bufs */
	u_int	m_mbxsize;	/* no. of bytes for mbufx */
	u_int	m_tbcount;	/* no. of trace buffers */
	u_int	m_tbindex;	/* next trace buffer */
#ifdef DIAGNOSTIC
#define	NMBTBUF	40
	struct m_tbuf {
		u_int	mt_type;	/* type plus KISA6 */
		u_int	mt_pc;
		u_int	mt_arg;
	} m_tbuf[NMBTBUF];
#else
#define	NMBTBUF	0
#endif
#endif
};

#ifdef KERNEL
#ifdef DIAGNOSTIC
#define	MBTRACE(type,arg)	mbtrace(type,arg)
#else
#define	MBTRACE(type,arg)
#endif

struct	mbuf	*mbcache[NMBCACHE];
struct	mbstat	mbstat;
#define	mbicache	mbstat.m_incache
int	m_want;
struct	mbuf *m_get(),*m_getclr(),*m_free(),*m_more(),*m_copy(),*m_pullup();
struct	mbuf *m_bget(),*m_bfree();
char	*m_sget();
#endif
