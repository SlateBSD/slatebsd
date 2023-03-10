/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)if_de.c	1.1 (2.10BSD Berkeley) 12/1/86
 *
 */

#include "de.h"
#if NDE > 0

/*
 * DEC DEUNA interface
 *
 *     Lou Salkind
 *     New York University
 *
 * TODO:
 *     timeout routine (get statistics)
 */

#include "param.h"
#include "../machine/seg.h"

#include "short_names.h"
#include "mbuf.h"
#include "protosw.h"
#include "socket.h"
#include "ioctl.h"
#include "errno.h"

#ifdef BSD2_10
# include "buf.h"
#endif

#include "../pdpuba/ubavar.h"

#include "../net/if.h"
#include "../net/netisr.h"
#include "../net/route.h"

#include "../netinet/in_systm.h"
#include "../netinet/in.h"
#include "../netinet/in_var.h"
#include "../netinet/ip.h"
#include "../netinet/if_ether.h"
#include "../netns/ns.h"
#include "../netns/ns_if.h"

#include "../vaxif/if_uba.h"

#define	ETHERP_IPTYPE	0x0800		/* IP protocol */
#define	ETHERP_ARPTYPE	0x0806		/* ARP protocol */

/*
 * The ETHERPUP_NTRAILER packet types starting at ETHERPUP_TRAIL have
 * (type-ETHERPUP_TRAIL)*512 bytes of data followed
 * by a PUP type (as given above) and then the (variable-length) header.
 */
#define	ETHERP_TRAIL	0x1000		/* Trailer PUP */
#define	ETHERP_NTRAILER	16

#define	ETHERMTU	1500
#define	ETHERMIN	(60-14)

#include "../vaxif/if_de.h"

#ifdef BSD2_10
/*
 * on a pdp11 we only have 8k of io space to allocate. This could
 * be increased but for now just reduce the number of buffers that we
 * use. (1500 + SLOP) * 5 ~= 6 + k?
 */
#define	 NXMT    2	/* number of transmit buffers (must be > 1) */
#define	 NRCV    3	/* number of receive buffers (must be > 1) */
#else
#define	 NXMT    2	/* number of transmit buffers */
#define	 NRCV    4	/* number of receive buffers (must be > 1) */
#endif

/* #define	 NTOT    (NXMT + NRCV) */

int	dedebug = 1;

int	deprobe(), deattach(), deintr(),
	deinit(),  deoutput(), deioctl(), dereset();

struct mbuf *deget();

u_short destd[] = { 0 };

struct uba_device *deinfo[NDE];

struct uba_driver dedriver =
	{ deprobe, 0, deattach, 0, destd, "de", deinfo };


struct deuba {
	u_short	ifu_hlen;		/* local net header length */
	struct	ifrw difu_r[NRCV];	/* receive information */
	struct	ifrw difu_w[NXMT];	/* transmit information */
	short	difu_flags;		/* used during uballoc's */
};

struct	in_addr arpmyaddr();
struct	arptab *arptnew();

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ds_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 * We also have, for each interface, a UBA interface structure, which
 * contains information about the UNIBUS resources held by the interface:
 * map registers, buffered data paths, etc.  Information is cached in this
 * structure for use by the if_uba.c routines in running the interface
 * efficiently.
 */
struct de_softc {
	struct  arpcom ds_ac;		/* Ethernet common part */
#define	 ds_if   ds_ac.ac_if		/* network-visible interface */
#define	 ds_addr ds_ac.ac_enaddr	/* hardware Ethernet address */

	int     ds_flags;
#define	 DSF_LOCK	 1		/* lock out destart */
#define	 DSF_RUNNING     2
#define	 DSF_SETADDR     4

	ubadr_t ds_ubaddr;		/* map info for incore structs */
	struct  deuba ds_deuba;		/* unibus resource structure */

	/* the following structures are always mapped in */
	struct  de_pcbb ds_pcbb;	/* port control block */
	struct  de_ring ds_xrent[NXMT];	/* transmit ring entrys */
	struct  de_ring ds_rrent[NRCV];	/* receive ring entrys */
	struct  de_udbbuf ds_udbbuf;	/* UNIBUS data buffer */
#define	 INCORE_BASE(p)  ((char *)&(p)->ds_pcbb)
#define	 RVAL_OFF(n)     ((char *)&de_softc[0].n - INCORE_BASE(&de_softc[0]))
#define	 LVAL_OFF(n)     ((char *)de_softc[0].n - INCORE_BASE(&de_softc[0]))
#define	 PCBB_OFFSET     RVAL_OFF(ds_pcbb)
#define	 XRENT_OFFSET    LVAL_OFF(ds_xrent)
#define	 RRENT_OFFSET    LVAL_OFF(ds_rrent)
#define	 UDBBUF_OFFSET   RVAL_OFF(ds_udbbuf)
#define	 INCORE_SIZE     RVAL_OFF(ds_xindex)
	/* end mapped area */

	int     ds_xindex;		/* UNA index into transmit chain */
	int     ds_rindex;		/* UNA index into receive chain */
	int     ds_xfree;		/* index for next transmit buffer */
	int     ds_nxmit;		/* # of transmits in progress */
} de_softc[NDE];

deprobe(reg)
	caddr_t reg;
{
#ifdef notdef
	register int br, cvec;	   /* r11, r10 value-result */
	register struct dedevice *addr = (struct dedevice *)reg;
	register i;

#ifdef lint
	br = 0; cvec = br; br = cvec;
	i = 0; derint(i); deintr(i);
#endif

	addr->pcsr0 = PCSR0_RSET;
	while ((addr->pcsr0 & PCSR0_INTR) == 0)
		;
	/* make board interrupt by executing a GETPCBB command */
	addr->pcsr0 = PCSR0_INTE;
	addr->pcsr2 = 0;
	addr->pcsr3 = 0;
	addr->pcsr0 = PCSR0_INTE|CMD_GETPCBB;
	DELAY(100000);
#endif
	return(1);
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.  We get the ethernet address here.
 */
deattach(ui)
	struct uba_device *ui;
{
	register struct de_softc *ds = &de_softc[ui->ui_unit];
	register struct ifnet *ifp = &ds->ds_if;
	register struct dedevice *addr = (struct dedevice *)ui->ui_addr;
	struct sockaddr_in *sin;
	int csr0;

	ifp->if_unit = ui->ui_unit;
	ifp->if_name = "de";
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags = IFF_BROADCAST;

	/*
	* Reset the board and temporarily map
	* the pcbb buffer onto the Unibus.
	*/
	addr->pcsr0 = PCSR0_RSET;
	(void)dewait(ui, "reset");

	ds->ds_ubaddr = uballoc(ui->ui_ubanum, (char *)&ds->ds_pcbb,
		sizeof (struct de_pcbb), 0);
	addr->pcsr2 = ds->ds_ubaddr & 0xffff;
	addr->pcsr3 = (ds->ds_ubaddr >> 16) & 0x3;
	addr->pclow = CMD_GETPCBB;
	(void)dewait(ui, "pcbb");

	ds->ds_pcbb.pcbb0 = FC_RDPHYAD;
	addr->pclow = CMD_GETCMD;
	(void)dewait(ui, "read addr");

	ubarelse(ui->ui_ubanum, &ds->ds_ubaddr);
	bcopy((caddr_t)&ds->ds_pcbb.pcbb2, (caddr_t)ds->ds_addr,
	   sizeof (ds->ds_addr));

	if (dedebug)
		printf( "de%d: hardware address %s\n",
			ui->ui_unit, ether_sprintf(ds->ds_addr)
		);

	ifp->if_init = deinit;
	ifp->if_output = deoutput;
	ifp->if_ioctl = deioctl;
	ifp->if_reset = dereset;
	ds->ds_deuba.difu_flags = UBA_CANTWAIT;

	if_attach(ifp);
}


/*
 * Reset of interface after UNIBUS reset.
 * If interface is on specified uba, reset its state.
 */
dereset(unit, uban)
	int unit, uban;
{
	register struct uba_device *ui;

	if (unit >= NDE || (ui = deinfo[unit]) == 0 || ui->ui_alive == 0 ||
	    ui->ui_ubanum != uban)
		return;
	printf(" de%d", unit);
	de_softc[unit].ds_if.if_flags &= ~IFF_RUNNING;
	de_softc[unit].ds_flags &= ~(DSF_LOCK | DSF_RUNNING);
	deinit(unit);
}

/*
 * Initialization of interface; clear recorded pending
 * operations, and reinitialize UNIBUS usage.
 */
deinit(unit)
	int unit;
{
	register struct de_softc *ds = &de_softc[unit];
	register struct uba_device *ui = deinfo[unit];
	register struct dedevice *addr;
	register struct ifrw *ifrw;
	struct ifnet *ifp = &ds->ds_if;
	struct sockaddr_in *sin;
	int s;
	struct de_ring *rp;
	ubadr_t incaddr;
	int csr0;

	/* not yet, if address still unknown */
	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;

	if (ifp->if_flags & IFF_RUNNING)
		return;

	if (de_ubainit(&ds->ds_deuba, ui->ui_ubanum,
	   sizeof (struct ether_header), (int)btoc(ETHERMTU)) == 0) { 
		printf("de%d: can't initialize\n", unit);
		ds->ds_if.if_flags &= ~IFF_UP;
		return;
	}
	ds->ds_ubaddr = uballoc(ui->ui_ubanum, INCORE_BASE(ds), INCORE_SIZE,0);
	addr = (struct dedevice *)ui->ui_addr;

	/* set the pcbb block address */
	incaddr = ds->ds_ubaddr + PCBB_OFFSET;
	addr->pcsr2 = incaddr & 0xffff;
	addr->pcsr3 = (incaddr >> 16) & 0x3;
	addr->pclow = CMD_GETPCBB;
	dewait(ui, "pcbb");

	/* set the transmit and receive ring header addresses */
	incaddr = ds->ds_ubaddr + UDBBUF_OFFSET;
	ds->ds_pcbb.pcbb0 = FC_WTRING;
	ds->ds_pcbb.pcbb2 = incaddr & 0xffff;
	ds->ds_pcbb.pcbb4 = (incaddr >> 16) & 0x3;

	incaddr = ds->ds_ubaddr + XRENT_OFFSET;
	ds->ds_udbbuf.b_tdrbl = incaddr & 0xffff;
	ds->ds_udbbuf.b_tdrbh = (incaddr >> 16) & 0x3;
	ds->ds_udbbuf.b_telen = sizeof (struct de_ring) / sizeof (short);
	ds->ds_udbbuf.b_trlen = NXMT;
	incaddr = ds->ds_ubaddr + RRENT_OFFSET;
	ds->ds_udbbuf.b_rdrbl = incaddr & 0xffff;
	ds->ds_udbbuf.b_rdrbh = (incaddr >> 16) & 0x3;
	ds->ds_udbbuf.b_relen = sizeof (struct de_ring) / sizeof (short);
	ds->ds_udbbuf.b_rrlen = NRCV;

	addr->pclow = CMD_GETCMD;
	dewait(ui, "wtring");

	/* initialize the mode - enable hardware padding */
	ds->ds_pcbb.pcbb0 = FC_WTMODE;
	/* let hardware do padding - set MTCH bit on broadcast */
	ds->ds_pcbb.pcbb2 = MOD_TPAD|MOD_HDX;
	addr->pclow = CMD_GETCMD;
	dewait(ui, "wtmode");

	/* set up the receive and transmit ring entries */
	ifrw = &ds->ds_deuba.difu_w[0];
	for (rp = &ds->ds_xrent[0]; rp < &ds->ds_xrent[NXMT]; rp++) {
		rp->r_segbl = ifrw->ifrw_info & 0xffff;
		rp->r_segbh = (ifrw->ifrw_info >> 16) & 0x3;
		rp->r_flags = 0;
		ifrw++;
	}
	ifrw = &ds->ds_deuba.difu_r[0];
	for (rp = &ds->ds_rrent[0]; rp < &ds->ds_rrent[NRCV]; rp++) {
		rp->r_slen = sizeof (struct de_buf);
		rp->r_segbl = ifrw->ifrw_info & 0xffff;
		rp->r_segbh = (ifrw->ifrw_info >> 16) & 0x3;
		rp->r_flags = RFLG_OWN;	  /* hang receive */
		ifrw++;
	}

	/* start up the board (rah rah) */
	s = splimp();
	ds->ds_rindex = ds->ds_xindex = ds->ds_xfree = 0;
	ds->ds_if.if_flags |= IFF_UP|IFF_RUNNING;
	destart(unit);			     /* queue output packets */
	addr->pclow = PCSR0_INTE;		 /* avoid interlock */
	ds->ds_flags |= DSF_RUNNING;
	if(ds->ds_flags & DSF_SETADDR)
		de_setaddr(ds->ds_addr, unit);
	addr->pclow = CMD_START | PCSR0_INTE;
	splx(s);

}

/*
 * Setup output on interface.
 * Get another datagram to send off of the interface queue,
 * and map it to the interface before starting the output.
 */
destart(unit)
	int unit;
{
	int len;
	struct uba_device *ui = deinfo[unit];
	struct dedevice *addr = (struct dedevice *)ui->ui_addr;
	register struct de_softc *ds = &de_softc[unit];
	register struct de_ring *rp;
	struct mbuf *m;
	register int nxmit;

	/*
	 * the following test is necessary, since
	 * the code is not reentrant and we have
	 * multiple transmission buffers.
	 */
	if (ds->ds_flags & DSF_LOCK)
		return;

	for (nxmit = ds->ds_nxmit; nxmit < NXMT; nxmit++) {
		IF_DEQUEUE(&ds->ds_if.if_snd, m);
		if (m == 0)
			break;
		rp = &ds->ds_xrent[ds->ds_xfree];
		if (rp->r_flags & XFLG_OWN)
			panic("deuna xmit in progress");
		len = deput(&ds->ds_deuba, ds->ds_xfree, m);
		rp->r_slen = len;
		rp->r_tdrerr = 0;
		rp->r_flags = XFLG_STP|XFLG_ENP|XFLG_OWN;

		ds->ds_xfree++;
		if (ds->ds_xfree >= NXMT)
			ds->ds_xfree = 0;
	}
	if (ds->ds_nxmit != nxmit) {
		ds->ds_nxmit = nxmit;
		if (ds->ds_flags & DSF_RUNNING)
			addr->pclow = PCSR0_INTE|CMD_PDMD;
	}
}

/*
 * Command done interrupt.
 */
deintr(unit)
	int unit;
{
	struct uba_device *ui = deinfo[unit];
	register struct dedevice *addr = (struct dedevice *)ui->ui_addr;
	register struct de_softc *ds = &de_softc[unit];
	register struct de_ring *rp;
	register struct ifrw *ifrw;
	short csr0;

	/* save flags right away - clear out interrupt bits */
	csr0 = addr->pcsr0;
	addr->pchigh = csr0 >> 8;

	ds->ds_flags |= DSF_LOCK;	/* prevent entering destart */

	/*
	 * if receive, put receive buffer on mbuf
	 * and hang the request again
	 */
	derecv(unit);

	/*
	 * Poll transmit ring and check status.
	 * Be careful about loopback requests.
	 * Then free buffer space and check for
	 * more transmit requests.
	 */
	for ( ; ds->ds_nxmit > 0; ds->ds_nxmit--) {
		rp = &ds->ds_xrent[ds->ds_xindex];
		if (rp->r_flags & XFLG_OWN)
			break;
		ds->ds_if.if_opackets++;
		ifrw = &ds->ds_deuba.difu_w[ds->ds_xindex];
		/* check for unusual conditions */
		if (rp->r_flags & (XFLG_ERRS|XFLG_MTCH|XFLG_ONE|XFLG_MORE)) {
			if (rp->r_flags & XFLG_ERRS) {
				/* output error */
				ds->ds_if.if_oerrors++;
				if (dedebug)
			printf("de%d: oerror, flags=%b tdrerr=%b (len=%d)\n",
				   unit, rp->r_flags, XFLG_BITS,
				   rp->r_tdrerr, XERR_BITS, rp->r_slen);
			} else if (rp->r_flags & XFLG_ONE) {
				/* one collision */
				ds->ds_if.if_collisions++;
			} else if (rp->r_flags & XFLG_MORE) {
				/* more than one collision */
				ds->ds_if.if_collisions += 2;   /* guess */
			} else if (rp->r_flags & XFLG_MTCH) {
				/* received our own packet */
				ds->ds_if.if_ipackets++;
				deread(ds, ifrw,
				   rp->r_slen - sizeof (struct ether_header));
			}
		}
		/* check if next transmit buffer also finished */
		ds->ds_xindex++;
		if (ds->ds_xindex >= NXMT)
			ds->ds_xindex = 0;
	}
	ds->ds_flags &= ~DSF_LOCK;
	destart(unit);

	if (csr0 & PCSR0_RCBI) {
		printf("de%d: buffer unavailable\n", unit);
		addr->pclow = PCSR0_INTE|CMD_PDMD;
	}
}

/*
 * Ethernet interface receiver interface.
 * If input error just drop packet.
 * Otherwise purge input buffered data path and examine 
 * packet to determine type.  If can't determine length
 * from type, then have to drop packet.  Othewise decapsulate
 * packet based on type and pass to type specific higher-level
 * input routine.
 */
derecv(unit)
	int unit;
{
	register struct de_softc *ds = &de_softc[unit];
	register struct de_ring *rp;
	int len;

	rp = &ds->ds_rrent[ds->ds_rindex];
	while ((rp->r_flags & RFLG_OWN) == 0) {
		ds->ds_if.if_ipackets++;

		len = (rp->r_lenerr&RERR_MLEN) - sizeof (struct ether_header)
			- 4;    /* don't forget checksum! */

		/* check for errors */
		if ((rp->r_flags & (RFLG_ERRS|RFLG_FRAM|RFLG_OFLO|RFLG_CRC)) ||
		   (rp->r_flags&(RFLG_STP|RFLG_ENP)) != (RFLG_STP|RFLG_ENP) ||
		   (rp->r_lenerr & (RERR_BUFL|RERR_UBTO|RERR_NCHN)) ||
		   len < ETHERMIN || len > ETHERMTU) {
			ds->ds_if.if_ierrors++;
			if (dedebug)
			printf("de%d: ierror, flags=%b lenerr=%b (len=%d)\n",
				unit, rp->r_flags, RFLG_BITS, rp->r_lenerr,
				RERR_BITS, len);
		} else
			deread(ds, &ds->ds_deuba.difu_r[ds->ds_rindex], len);

		/* hang the receive buffer again */
		rp->r_lenerr = 0;
		rp->r_flags = RFLG_OWN;

		/* check next receive buffer */
		ds->ds_rindex++;
		if (ds->ds_rindex >= NRCV)
			ds->ds_rindex = 0;
		rp = &ds->ds_rrent[ds->ds_rindex];
	}
}

/*
 * Pass a packet to the higher levels.
 * We deal with the trailer protocol here.
 */
deread(ds, ifrw, len)
	register struct de_softc *ds;
	struct ifrw *ifrw;
	int len;
{
	struct ether_header *eh;
	struct mbuf *m;
	int off, resid, s;
	register struct ifqueue *inq;
	mapinfo	map;

	/*
	 * Deal with trailer protocol: if type is PUP trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */

	savemap(map);

#ifdef BSD2_10
	mapseg5(ifrw->ifrw_click, MBMAPSIZE);
	eh = (struct ether_header *)MBX;
#else !BSD2_10
	eh = (struct ether_header *)ifrw->ifrw_addr;
#endif BSD2_10

	eh->ether_type = ntohs((u_short)eh->ether_type);

#define   dedataaddr(eh, off, type)	((type)(((caddr_t)((eh)+1)+(off))))

	if (eh->ether_type >= ETHERP_TRAIL &&
	   eh->ether_type < ETHERP_TRAIL+ETHERP_NTRAILER) {

		off = (eh->ether_type - ETHERP_TRAIL) * 512;

		if (off >= ETHERMTU) {
			restormap(map);
			return;	  /* sanity */
		}

		eh->ether_type = ntohs(*dedataaddr(eh, off, u_short *));
		resid = ntohs(*(dedataaddr(eh, off+2, u_short *)));

		if (off + resid > len) {
			restormap(map);
			return;	  /* sanity */
		}

		len = off + resid;

	} else
		off = 0;

	if (len == 0) {
		restormap(map);
		return;
	}

	/*
	* Pull packet off interface.  Off is nonzero if packet
	* has trailing header; deget will then force this header
	* information to be at the front, but we still have to drop
	* the type and length which are at the front of any trailer data.
	*/
	m = deget(&ds->ds_deuba, ifrw, len, off, ds->ds_if);
	if (m == 0) {
		restormap(map);
		return;
	}

	s = eh->ether_type;	/* reuse s, to avoid MAPSAVE();	*/

	/* scarey, but apparently correct ... */
	if (off) {
		struct ifnet *ifp;

		ifp = *(mtod(m, struct ifnet **));
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
		*(mtod(m, struct ifnet **)) = ifp;
	}

	switch (s) {

#ifdef INET
	case ETHERP_IPTYPE:
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;

	case ETHERP_ARPTYPE:
		arpinput(&ds->ds_ac, m);
		restormap(map);
		return;
#endif
	default:
		m_freem(m);
		restormap(map);
		return;
	}

	s = splimp();
	if (IF_QFULL(inq)) {
		if(dedebug)
			printf("de0: que full - drop\n");
		IF_DROP(inq);
		splx(s);
		m_freem(m);
		restormap(map);
		return;
	}
	IF_ENQUEUE(inq, m);
	splx(s);

	restormap(map);
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
deoutput(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	int type, s, error;
	u_char	edst[6];
	struct in_addr idst;
	register struct de_softc *ds = &de_softc[ifp->if_unit];
	register struct mbuf *m = m0;
	register struct ether_header *eh;
	register int off;
	int usetrailers;
	segm save5;

	saveseg5(save5);

	if((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		goto bad;
	}

	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		if (!arpresolve(&ds->ds_ac, m, &idst, &edst, &usetrailers)) {
			restorseg5(save5);
			return (0);     /* if not yet resolved */
		}

		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;

/* this is totally wrong; use usetrailers; KB */
		/* need per host negotiation */
		if ((ifp->if_flags & IFF_NOTRAILERS) == 0)
		if (off > 0 && (off & 0x1ff) == 0 &&
		   m->m_off >= MMINOFF + 2 * sizeof (u_short)
		) {
			type = ETHERP_TRAIL + (off>>9);
			m->m_off -= 2 * sizeof (u_short);
			m->m_len += 2 * sizeof (u_short);
			*mtod(m, u_short *) = htons((u_short)ETHERP_IPTYPE);
			*(mtod(m, u_short *) + 1) = htons((u_short)m->m_len);
			goto gottrailertype;
		}
		type = ETHERP_IPTYPE;
		off = 0;
		goto gottype;
#endif

	case AF_UNSPEC:
		eh = (struct ether_header *)dst->sa_data;
		bcopy( (caddr_t)eh->ether_dhost, (caddr_t)edst, sizeof (edst));
		type = eh->ether_type;
		goto gottype;

	default:
		printf("de%d: can't handle af%d\n", ifp->if_unit,
			dst->sa_family);
		error = EAFNOSUPPORT;
		goto bad;
	}

gottrailertype:
	/*
	* Packet to be sent as trailer: move first packet
	* (control information) to end of chain.
	*/
	while (m->m_next)
		m = m->m_next;
	m->m_next = m0;
	m = m0->m_next;
	m0->m_next = 0;
	m0 = m;

gottype:
	/*
	* Add local net header.  If no space in first mbuf,
	* allocate another.
	*/
	if (m->m_off > MMAXOFF ||
	   MMINOFF + sizeof (struct ether_header) > m->m_off) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct ether_header);
	} else {
		m->m_off -= sizeof (struct ether_header);
		m->m_len += sizeof (struct ether_header);
	}
	eh = mtod(m, struct ether_header *);
	eh->ether_type = htons((u_short)type);
	bcopy( (caddr_t)edst, (caddr_t)eh->ether_dhost, sizeof (edst));
	/* DEUNA fills in source address */

	/*
	* Queue message on interface, and start output if interface
	* not yet active.
	*/
	s = splimp();
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		splx(s);
		m_freem(m);
		return (ENOBUFS);
	}
	IF_ENQUEUE(&ifp->if_snd, m);
	destart(ifp->if_unit);
	splx(s);
	restorseg5(save5);
	return (0);

bad:
	m_freem(m0);
	restorseg5(save5);
	return (error);
}


/*
 * Routines supporting UNIBUS network interfaces.
 */

de_ubainit(ifu, uban, hlen, nmr)
	register struct deuba *ifu;
	int uban, hlen, nmr;
{
	register caddr_t cp, dp;
	register struct ifrw *ifrw;
	int i, ncl;

	if (ifu->difu_r[0].ifrw_click)
		return(1);
	nmr = ctob(nmr);  /* convert clicks back to bytes */
	nmr += hlen;
	for(i = 0 ; i < NRCV ; i++){
		ifu->difu_r[i].ifrw_click = m_ioget(nmr);
		if(ifu->difu_r[i].ifrw_click == 0){
			ifu->difu_r[0].ifrw_click = 0;
			if(i)
				printf("de: lost some space\n"); /* XXX */
			return(0);
		}
	}
	for(i = 0 ; i < NXMT ; i++){
		ifu->difu_w[i].ifrw_click = m_ioget(nmr);
		if(ifu->difu_w[i].ifrw_click == 0){
			ifu->difu_w[0].ifrw_click = 0;
			ifu->difu_r[0].ifrw_click = 0;
			if(i)
				printf("de: lost some space\n"); /* XXX */
			return(0);
		}
	}
	for(i = 0 ; i < NRCV ; i++)
		ifu->difu_r[i].ifrw_info =
				ubmalloc(0,ifu->difu_r[i].ifrw_click,nmr,0);
	for(i = 0 ; i < NXMT ; i++)
		ifu->difu_w[i].ifrw_info =
				ubmalloc(0,ifu->difu_w[i].ifrw_click,nmr,0);
	ifu->ifu_hlen = hlen;
	return (1);
}

/*
 * Pull read data off a interface.
 * Len is length of data, with local net header stripped.
 * Off is non-zero if a trailer protocol was used, and
 * gives the offset of the trailer information.
 * We copy the trailer information and then all the normal
 * data into mbufs.  When full cluster sized units are present
 * on the interface on cluster boundaries we can get them more
 * easily by remapping, and take advantage of this here.
 */
struct mbuf *
deget(ifu, ifrw, totlen, off0, ifp)
	register struct deuba *ifu;
	register struct ifrw *ifrw;
	int totlen, off0;
	struct ifnet *ifp;
{
	struct mbuf *top, **mp, *m;
	int off = off0, len;
#ifdef BSD2_10
	register caddr_t cp = (caddr_t)ifu->ifu_hlen;
#else !BSD2_10
	register caddr_t cp = ifu->ifu_hlen;
#endif BSD2_10
	u_int click;

	top = 0;
	mp = &top;
	click = ifrw->ifrw_click;
	while (totlen > 0) {
		MGET(m, 0, MT_DATA);
		if (m == 0)
			goto bad;
		if (off) {
			len = totlen - off;
#ifdef BSD2_10
			cp = (caddr_t)(ifu->ifu_hlen + off);
#else !BSD2_10
			cp = ifu->ifu_hlen + off;
#endif BSD2_10
		} else
			len = totlen;

		m->m_off = MMINOFF;
		if (ifp) {
			/*
			 *	Leave room for ifp.
			 */
			m->m_len = MIN(MLEN - sizeof(ifp), len);
			m->m_off += sizeof(ifp);
		} else
			m->m_len = MIN(MLEN, len);

		copyv(click, cp, m->m_click, m->m_off, (u_int)m->m_len);
		cp += m->m_len;
		*mp = m;
		mp = &m->m_next;
		if (off) {
			/* sort of an ALGOL-W style for statement... */
			off += m->m_len;
			if (off == totlen) {
#ifdef BSD2_10
				cp = (caddr_t)ifu->ifu_hlen;
#else !BSD2_10
				cp = ifu->ifu_hlen;
#endif BSD2_10
				off = 0;
				totlen = off0;
			}
		} else
			totlen -= m->m_len;

		if(ifp) {
			/*
			 *	Prepend interface pointer to first mbuf.
			 */
			m->m_len += sizeof(ifp);
			m->m_off -= sizeof(ifp);
			MAPSAVE();
			*(mtod(m, struct ifnet **)) = ifp;
			MAPREST();
			ifp = (struct ifnet *)0;
		}
	}

	return (top);
bad:
	m_freem(top);
	return (0);
}

/*
 * Map a chain of mbufs onto a network interface
 * in preparation for an i/o operation.
 */
deput(ifu, n, m)
	struct deuba *ifu;
	int n;
	register struct mbuf *m;
{
	register u_short off,click;
	struct mbuf *mp;

	click = ifu->difu_w[n].ifrw_click;
	off = 0;
	while (m) {
		copyv(m->m_click,m->m_off,click,off,(u_int)m->m_len);
		off += m->m_len;
		MFREE(m, mp);
		m = mp;
	}
	return (off);
}

/*
 * Process an ioctl request.
 */
deioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	register struct de_softc *ds = &de_softc[ifp->if_unit];
	int s = splimp(), error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		deinit(ifp->if_unit);

		switch (ifa->ifa_addr.sa_family) {
#ifdef INET
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif
#ifdef NS
		case AF_NS:
		    {
			register struct ns_addr *ina = &(IA_SNS(ifa)->sns_addr);
			
			if (ns_nullhost(*ina))
				ina->x_host = *(union ns_host *)(ds->ds_addr);
			else
				de_setaddr(ina->x_host.c_host,ifp->if_unit);
			break;
		    }
#endif
		}
		break;

	case SIOCSIFFLAGS:
		if ((ifp->if_flags & IFF_UP) == 0 &&
		    ds->ds_flags & DSF_RUNNING) {
			((struct dedevice *)
			   (deinfo[ifp->if_unit]->ui_addr))->pclow = PCSR0_RSET;
			ds->ds_flags &= ~(DSF_LOCK | DSF_RUNNING);
		} else if (ifp->if_flags & IFF_UP &&
		    (ds->ds_flags & DSF_RUNNING) == 0)
			deinit(ifp->if_unit);
		break;

	default:
		error = EINVAL;
	}

	splx(s);
	return (error);
}


/*
 * set ethernet address for unit
 */
de_setaddr(physaddr, unit)
	u_char *physaddr;
	int unit;
{
	register struct de_softc *ds = &de_softc[unit];
	struct uba_device *ui = deinfo[unit];
	register struct dedevice *addr= (struct dedevice *)ui->ui_addr;
	
	if (! (ds->ds_flags & DSF_RUNNING))
		return;
		
	bcopy(physaddr, &ds->ds_pcbb.pcbb2, 6);
	ds->ds_pcbb.pcbb0 = FC_WTPHYAD;
	addr->pclow = PCSR0_INTE|CMD_GETCMD;
	if (dewait(ui, "address change") == 0) {
		ds->ds_flags |= DSF_SETADDR;
		bcopy(physaddr, ds->ds_addr, 6);
	}
}


/*
 * Await completion of the named function
 * and check for errors.
 */
dewait(ui, fn)
	register struct uba_device *ui;
	char *fn;
{
	register struct dedevice *addr = (struct dedevice *)ui->ui_addr;
	register csr0;

	while ((addr->pcsr0 & PCSR0_INTR) == 0)
		;
	csr0 = addr->pcsr0;
	addr->pchigh = csr0 >> 8;
	if (csr0 & PCSR0_PCEI)
		printf("de%d: %s failed, csr0=%b csr1=%b\n", 
		    ui->ui_unit, fn, csr0, PCSR0_BITS, 
		    addr->pcsr1, PCSR1_BITS);
	return (csr0 & PCSR0_PCEI);
}

#endif
