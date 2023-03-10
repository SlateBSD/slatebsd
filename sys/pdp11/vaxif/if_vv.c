/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)if_vv.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#ifdef AUTOCONFIG
#include "../machine/autoconfig.h"
vvprobe(addr)
struct vvdevice	*addr;
{
	extern int	errno;

	errno = 0;
	grab(addr);
	return(errno ? ACP_NXDEV : ACP_EXISTS);
}
#else !AUTOCONFIG
#include "vv.h"
#if NVV > 0

/*
 * Proteon 10 Meg Ring Driver.
 * This device is called "vv" because its "real name",
 * V2LNI won't work if shortened to the obvious "v2".
 * Hence the subterfuge.
 *
 */

#include "param.h"
#include "../machine/seg.h"

#include "systm.h"
#include "mbuf.h"
#include "buf.h"
#include "protosw.h"
#include "socket.h"
#include "pdpuba/ubavar.h"
#include "ioctl.h"
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#include <vaxif/if_vv.h>
#include <vaxif/if_uba.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <net/route.h>
#include <errno.h>

/*
 * N.B. - if WIRECENTER is defined wrong, it can well break
 * the hardware!!
 */
#define	WIRECENTER

#ifdef WIRECENTER
#define	VV_CONF	VV_HEN		/* drive wire center relay */
#else
#define	VV_CONF	VV_STE		/* allow operation without wire center */
#endif

#define	VVMTU	(1024+512)
#define VVMRU	(1024+512+16)	/* space for trailer */

int	vv_tracehdr = 0;	/* 1 => trace headers (slowly!!) */
int	vv_logioerrors = 1;	/* 1 => log all read errors */

#define vvtracehdr	if (vv_tracehdr) vvprt_hdr

int	vvprobe(), vvattach(), vvreset(), vvinit();
int	vvstart(), vvxint(), vvwatchdog();
int	vvrint(), vvoutput(), vvioctl(), vvsetaddr();
#ifdef BSD2_10
u_long	vvidentify();
#else !BSD2_10
int	vvidentify();
#endif BSD2_10
struct	uba_device *vvinfo[NVV];
u_short vvstd[] = { 0 };
struct	uba_driver vvdriver =
	{ vvprobe, 0, vvattach, 0, vvstd, "vv", vvinfo };
#define	VVUNIT(x)	minor(x)

/*
 * Software status of each interface.
 *
 * Each interface is referenced by a network interface structure,
 * vs_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 * We also have, for each interface, a UBA interface structure, which
 * contains information about the UNIBUS resources held by the interface:
 * map registers, buffered data paths, etc.  Information is cached in this
 * structure for use by the if_uba.c routines in running the interface
 * efficiently.
 */
struct	vv_softc {
	struct	ifnet vs_if;		/* network-visible interface */
	struct	ifuba vs_ifuba;		/* UNIBUS resources */
	short	vs_oactive;		/* is output active */
	short	vs_olen;		/* length of last output */
	u_short	vs_lastx;		/* address of last packet sent */
	u_short	vs_lastr;		/* address of last packet received */
	short	vs_tries;		/* transmit current retry count */
	short	vs_init;		/* number of ring inits */
	short	vs_nottaken;		/* number of packets refused */
	short	vs_timeouts;		/* number of transmit timeouts */
} vv_softc[NVV];

vvprobe(reg)
	caddr_t reg;
{
#ifndef BSD2_10
	register int br, cvec;
	register struct vvreg *addr;

#ifdef lint
	br = 0; cvec = br; br = cvec;
#endif
	addr = (struct vvreg *)reg;
	/* reset interface, enable, and wait till dust settles */
	addr->vvicsr = VV_RST;
	addr->vvocsr = VV_RST;
	DELAY(100000);
	/* generate interrupt by doing 1 word DMA from 0 in uba space!! */
	addr->vvocsr = VV_IEN;		/* enable interrupt */
	addr->vvoba = 0;		/* low 16 bits */
	addr->vvoea = 0;		/* extended bits */
	addr->vvowc = -1;		/* for 1 word */
	addr->vvocsr |= VV_DEN;		/* start the DMA */
	DELAY(100000);
	addr->vvocsr = 0;
	if (cvec && cvec != 0x200)
		cvec -= 4;		/* backup so vector => receive */
#endif !BSD2_10
	return(1);
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
vvattach(ui)
	struct uba_device *ui;
{
	register struct vv_softc *vs;

	vs = &vv_softc[ui->ui_unit];
	vs->vs_if.if_unit = ui->ui_unit;
	vs->vs_if.if_name = "vv";
	vs->vs_if.if_mtu = VVMTU;
	vs->vs_if.if_init = vvinit;
	vs->vs_if.if_ioctl = vvioctl;
	vs->vs_if.if_output = vvoutput;
	vs->vs_if.if_timer = 0;
	vs->vs_if.if_watchdog = vvwatchdog;
#ifdef BSD2_10
	vs->vs_if.if_ubareset = vvreset;
#else !BSD2_10
	vs->vs_if.if_reset = vvreset;
#endif BSD2_10
	vs->vs_ifuba.ifu_flags = UBA_CANTWAIT | UBA_NEEDBDP | UBA_NEED16;
#if defined(VAX750)
	/* don't chew up 750 bdp's */
	if (cpu == VAX_750 && ui->ui_unit > 0)
		vs->vs_ifuba.ifu_flags &= ~UBA_NEEDBDP;
#endif
	if_attach(&vs->vs_if);
}

/*
 * Reset of interface after UNIBUS reset.
 * If interface is on specified uba, reset its state.
 */
vvreset(unit, uban)
	int unit, uban;
{
	register struct uba_device *ui;

	if (unit >= NVV || (ui = vvinfo[unit]) == 0 || ui->ui_alive == 0 ||
	    ui->ui_ubanum != uban)
		return;
	printf(" vv%d", unit);
	vvinit(unit);
}

/*
 * Initialization of interface; clear recorded pending
 * operations, and reinitialize UNIBUS usage.
 */
vvinit(unit)
	int unit;
{
	register struct vv_softc *vs;
	register struct uba_device *ui;
	register struct vvreg *addr;
	register struct sockaddr_in *sin;
	register int s;
#ifdef BSD2_10
	ubadr_t ubainfo;
#else !BSD2_10
	register int ubainfo;
#endif BSD2_10

	vs = &vv_softc[unit];
	ui = vvinfo[unit];
	sin = (struct sockaddr_in *)&vs->vs_if.if_addr;
	/*
	 * If the network number is still zero, we've been
	 * called too soon.
	 */
	if (in_netof(sin->sin_addr) == 0)
		return;
	addr = (struct vvreg *)ui->ui_addr;
	if (if_ubainit(&vs->vs_ifuba, ui->ui_ubanum,
	    sizeof (struct vv_header), (int)btoc(VVMTU)) == 0) {
		printf("vv%d: can't initialize, if_ubainit() failed\n", unit);
		vs->vs_if.if_flags &= ~IFF_UP;
		return;
	}
	/*
	 * Now that the uba is set up, figure out our address and
	 * update complete our host address.
	 */
#ifdef BSD2_10
	{
		segm ZZMAP;
		saveseg5(ZZMAP);
		if ((vs->vs_if.if_host[0] = vvidentify(unit)) == 0) {
			vs->vs_if.if_flags &= ~IFF_UP;
			restorseg5(ZZMAP);
			return;
		}
		printf("vv%d: host %D\n", unit, vs->vs_if.if_host[0]);
		restorseg5(ZZMAP);
	}
#else !BSD2_10
		if ((vs->vs_if.if_host[0] = vvidentify(unit)) == 0) {
			vs->vs_if.if_flags &= ~IFF_UP;
			return;
		}
		printf("vv%d: host %d\n", unit, vs->vs_if.if_host[0]);
#endif BSD2_10
	sin->sin_addr = if_makeaddr(vs->vs_if.if_net, vs->vs_if.if_host[0]);
	/*
	 * Reset the interface, and join the ring
	 */
	addr->vvocsr = VV_RST | VV_CPB;		/* clear packet buffer */
	addr->vvicsr = VV_RST | VV_CONF;	/* close logical relay */
	DELAY(500000);				/* let contacts settle */
	vs->vs_init = 0;
	vs->vs_nottaken = 0;
	vs->vs_timeouts = 0;
	vs->vs_lastx = 256;			/* an invalid address */
	vs->vs_lastr = 256;			/* an invalid address */
	/*
	 * Hang a receive and start any
	 * pending writes by faking a transmit complete.
	 */
	s = splimp();
	ubainfo = vs->vs_ifuba.ifu_r.ifrw_info;
	addr->vviba = (u_short)ubainfo;
	addr->vviea = (u_short)(ubainfo >> 16);
	addr->vviwc = -(sizeof (struct vv_header) + VVMTU) >> 1;
	addr->vvicsr = VV_IEN | VV_CONF | VV_DEN | VV_ENB;
	vs->vs_oactive = 1;
	vs->vs_if.if_flags |= IFF_UP | IFF_RUNNING;
	vvxint(unit);
	splx(s);
	if_rtinit(&vs->vs_if, RTF_UP);
}

/*
 * Return our host address.
 */
#ifdef BSD2_10
u_long
#endif BSD2_10
vvidentify(unit)
	int unit;
{
	register struct vv_softc *vs;
	register struct uba_device *ui;
	register struct vvreg *addr;
	register struct mbuf *m;
	register struct vv_header *v;
	register int attempts, waitcount;
#ifdef BSD2_10
	ubadr_t ubainfo;
#else !BSD2_10
	register int ubainfo;
#endif BSD2_10

	/*
	 * Build a multicast message to identify our address
	 */
	vs = &vv_softc[unit];
	ui = vvinfo[unit];
	addr = (struct vvreg *)ui->ui_addr;
	attempts = 0;		/* total attempts, including bad msg type */
#ifdef BSD2_10
	m = m_get(M_DONTWAIT);
#else !BSD2_10
	m = m_get(M_DONTWAIT, MT_HEADER);
#endif BSD2_10
	if (m == NULL) {
		printf("vv%d: can't initialize, m_get() failed\n", unit);
		return (0);
	}
	m->m_next = 0;
	m->m_off = MMINOFF;
	m->m_len = sizeof(struct vv_header);
	v = mtod(m, struct vv_header *);
	v->vh_dhost = VV_BROADCAST;	/* multicast destination address */
	v->vh_shost = 0;		/* will be overwritten with ours */
	v->vh_version = RING_VERSION;
	v->vh_type = RING_WHOAMI;
	v->vh_info = 0;
	/* map xmit message into uba */
	vs->vs_olen =  if_wubaput(&vs->vs_ifuba, m);
	/*
	 * Reset interface, establish Digital Loopback Mode, and
	 * send the multicast (to myself) with Input Copy enabled.
	 */
retry:
	ubainfo = vs->vs_ifuba.ifu_r.ifrw_info;
	addr->vvicsr = VV_RST;
	addr->vviba = (u_short) ubainfo;
	addr->vviea = (u_short) (ubainfo >> 16);
	addr->vviwc = -(sizeof (struct vv_header) + VVMTU) >> 1;
	addr->vvicsr = VV_STE | VV_DEN | VV_ENB | VV_LPB;

	/* let flag timers fire so ring will initialize */
	DELAY(2000000);			/* about 2 SECONDS on a 780!! */

	addr->vvocsr = VV_RST | VV_CPB;	/* clear packet buffer */
	ubainfo = vs->vs_ifuba.ifu_w.ifrw_info;
	addr->vvoba = (u_short) ubainfo;
	addr->vvoea = (u_short) (ubainfo >> 16);
	addr->vvowc = -((vs->vs_olen + 1) >> 1);
	addr->vvocsr = VV_CPB | VV_DEN | VV_INR | VV_ENB;
	/*
	 * Wait for receive side to finish.
	 * Extract source address (which will be our own),
	 * and post to interface structure.
	 */
	DELAY(10000);
	for (waitcount = 0; (addr->vvicsr & VV_RDY) == 0; waitcount++) {
		if (waitcount < 10) {
			DELAY(1000);
			continue;
		}
		if (attempts++ < VVIDENTRETRY)
			goto retry;
		break;
	}
	/* deallocate mbuf used for send packet */
	if (vs->vs_ifuba.ifu_xtofree) {
		m_freem(vs->vs_ifuba.ifu_xtofree);
		vs->vs_ifuba.ifu_xtofree = 0;
	}
	if (attempts >= VVIDENTRETRY) {
		printf("vv%d: can't initialize after %d tries, icsr = %b\n",
		    unit, VVIDENTRETRY, 0xffff&(addr->vvicsr), VV_IBITS);
		return ((u_long)0);
	}
	m = if_rubaget(&vs->vs_ifuba, sizeof(struct vv_header), 0);
	if (m != NULL)
		m_freem(m);
	/*
	 * Check message type before we believe the source host address
	 */
#ifdef BSD2_10
	mapseg5(vs->vs_ifuba.ifu_r.ifrw_click, MBMAPSIZE);
	v = (struct vv_header *) MBX;
	if (v->vh_type != (u_char)RING_WHOAMI)
		goto retry;
	return((u_long)v->vh_shost);
#else !BSD2_10
	v = (struct vv_header *)(vs->vs_ifuba.ifu_r.ifrw_addr);
	if (v->vh_type != RING_WHOAMI)
		goto retry;
	return(v->vh_shost);
#endif BSD2_10
}

/*
 * Start or restart output on interface.
 * If interface is active, this is a retransmit, so just
 * restuff registers and go.
 * If interface is not already active, get another datagram
 * to send off of the interface queue, and map it to the interface
 * before starting the output.
 */
vvstart(dev)
	dev_t dev;
{
	register struct uba_device *ui;
	register struct vv_softc *vs;
	register struct vvreg *addr;
	register struct mbuf *m;
	register int unit, dest, s;
#ifdef BSD2_10
	ubadr_t ubainfo;
	segm save5;
#else !BSD2_10
	register int ubainfo;
#endif BSD2_10

	unit = VVUNIT(dev);
	ui = vvinfo[unit];
	vs = &vv_softc[unit];
	if (vs->vs_oactive)
		goto restart;
	/*
	 * Not already active: dequeue another request
	 * and map it to the UNIBUS.  If no more requests,
	 * just return.
	 */
	s = splimp();
	IF_DEQUEUE(&vs->vs_if.if_snd, m);
	splx(s);
	if (m == NULL) {
		vs->vs_oactive = 0;
		return;
	}
#ifdef BSD2_10
	saveseg5(save5);
#endif BSD2_10
	dest = mtod(m, struct vv_header *)->vh_dhost;
#ifdef BSD2_10
	restorseg5(save5);
#endif BSD2_10
	vs->vs_olen = if_wubaput(&vs->vs_ifuba, m);
	vs->vs_lastx = dest;
restart:
	/*
	 * Have request mapped to UNIBUS for transmission.
	 * Purge any stale data from this BDP, and start the output.
	 *
	 * Make sure this packet will fit in the interface.
	 */
	if (vs->vs_olen > VVMTU + sizeof (struct vv_header)) {
		printf("vv%d vs_olen: %d > VVMTU\n", unit, vs->vs_olen);
		panic("vvdriver vs_olen botch");
	}
	addr = (struct vvreg *)ui->ui_addr;
	ubainfo = vs->vs_ifuba.ifu_w.ifrw_info;
	addr->vvoba = (u_short) ubainfo;
	addr->vvoea = (u_short) (ubainfo >> 16);
	addr->vvowc = -((vs->vs_olen + 1) >> 1);
	addr->vvocsr = VV_IEN | VV_CPB | VV_DEN | VV_INR | VV_ENB;
	vs->vs_if.if_timer = VVTIMEOUT;
	vs->vs_oactive = 1;
}

/*
 * VVLNI transmit interrupt
 * Start another output if more data to send.
 */
vvxint(unit)
	int unit;
{
	register struct uba_device *ui;
	register struct vv_softc *vs;
	register struct vvreg *addr;
	register int oc;

	ui = vvinfo[unit];
	vs = &vv_softc[unit];
	vs->vs_if.if_timer = 0;
	addr = (struct vvreg *)ui->ui_addr;
	oc = 0xffff & (addr->vvocsr);
	if (vs->vs_oactive == 0) {
		printf("vv%d: stray interrupt vvocsr = %b\n", unit,
		    oc, VV_OBITS);
		return;
	}
	if (oc &  (VV_OPT | VV_RFS)) {
		vs->vs_if.if_collisions++;
		if (vs->vs_tries++ < VVRETRY) {
			if (oc & VV_OPT)
				vs->vs_init++;
			if (oc & VV_RFS)
				vs->vs_nottaken++;
			vvstart(unit);		/* restart this message */
			return;
		}
		if (oc & VV_OPT)
			printf("vv%d: output timeout\n", unit);
	}
	vs->vs_if.if_opackets++;
	vs->vs_oactive = 0;
	vs->vs_tries = 0;
	if (oc & VVXERR) {
		vs->vs_if.if_oerrors++;
		if (vv_logioerrors && vs->vs_if.if_flags & IFF_DEBUG)
		printf("vv%d: VVXERR, vvocsr = %b\n", unit,
			0xffff & oc, VV_OBITS);
	}
	if (vs->vs_ifuba.ifu_xtofree) {
		m_freem(vs->vs_ifuba.ifu_xtofree);
		vs->vs_ifuba.ifu_xtofree = 0;
	}
#ifdef BSD2_10
	if (vs->vs_if.if_snd.ifq_head == 0) {
		vs->vs_lastx = 256;		/* an invalid address */
		return;
	}
#endif BSD2_10
	vvstart(unit);
}

/*
 * Transmit watchdog timer routine.
 * This routine gets called when we lose a transmit interrupt.
 * The best we can do is try to restart output.
 */
vvwatchdog(unit)
	int unit;
{
	register struct vv_softc *vs;
	register int s;

	vs = &vv_softc[unit];
	if (vs->vs_if.if_flags & IFF_DEBUG)
		printf("vv%d: lost a transmit interrupt.\n", unit);
	vs->vs_timeouts++;
	s = splimp();
	vvstart(unit);
	splx(s);
}

/*
 * V2lni interface receiver interrupt.
 * If input error just drop packet.
 * Otherwise purge input buffered data path and examine
 * packet to determine type.  If can't determine length
 * from type, then have to drop packet.  Otherwise decapsulate
 * packet based on type and pass to type specific higher-level
 * input routine.
 */
vvrint(unit)
	int unit;
{
	register struct vv_softc *vs;
	register struct vvreg *addr;
	register struct vv_header *vv;
	register struct ifqueue *inq;
	register struct mbuf *m;
	int len, off, s;
#ifdef BSD2_10
	ubadr_t ubainfo;
	segm save5;
#else !BSD2_10
	int ubainfo;
#endif BSD2_10
	short resid;

#ifdef BSD2_10
	saveseg5(save5);
#endif BSD2_10
	vs = &vv_softc[unit];
	vs->vs_if.if_ipackets++;
	addr = (struct vvreg *)vvinfo[unit]->ui_addr;
	if (addr->vvicsr & VVRERR) {
		if (vv_logioerrors && vs->vs_if.if_flags & IFF_DEBUG)
			printf("vv%d: VVRERR, vvicsr = %b\n", unit,
			    0xffff&(addr->vvicsr), VV_IBITS);
		goto dropit;
	}

	/*
	 * Get packet length from word count residue
	 *
	 * Compute header offset if trailer protocol
	 *
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; if_rubaget will then force this header
	 * information to be at the front.  The vh_info field
	 * carries the offset to the trailer data in trailer
	 * format packets.
	 */
#ifdef BSD2_10
	mapseg5(vs->vs_ifuba.ifu_r.ifrw_click, MBMAPSIZE);
	vv = (struct vv_header *) MBX;
#else !BSD2_10
	vv = (struct vv_header *)(vs->vs_ifuba.ifu_r.ifrw_addr);
#endif BSD2_10
	vvtracehdr("vi", vv);
	resid = addr->vviwc;
	if (resid)
		resid |= 0176000;		/* ugly!!!! */
	len = (((sizeof (struct vv_header) + VVMRU) >> 1) + resid) << 1;
	len -= sizeof(struct vv_header);
	if (len > VVMRU || len <= 0) {
		if (vv_logioerrors && vs->vs_if.if_flags & IFF_DEBUG)
			printf("vv%d: len too big, len = %d, vvicsr = %b\n",
			    unit, len, 0xffff&(addr->vvicsr), VV_IBITS);
		goto dropit;
	}
#define	vvdataaddr(vv, off, type)	((type)(((caddr_t)((vv)+1)+(off))))
	if (vv->vh_type >= RING_IPTrailer &&
	     vv->vh_type < RING_IPTrailer+RING_IPNTrailer) {
		off = (vv->vh_type - RING_IPTrailer) * 512;
		if (off > VVMTU) {
			if (vv_logioerrors && vs->vs_if.if_flags & IFF_DEBUG)
				printf("vv%d: VVMTU, off = %d, vvicsr = %b\n",
				    unit, off, 0xffff&(addr->vvicsr), VV_IBITS);
			goto dropit;
		}
		vv->vh_type = *vvdataaddr(vv, off, u_short *);
		resid = *(vvdataaddr(vv, off+2, u_short *));
		if (off + resid > len) {
			if (vv_logioerrors && vs->vs_if.if_flags & IFF_DEBUG)
				printf(
				    "vv%d: off = %d, resid = %d, vvicsr = %b\n",
				    unit, off, resid,
				    0xffff&(addr->vvicsr), VV_IBITS);
			goto dropit;
		}
		len = off + resid;
	} else
		off = 0;
	if (len == 0) {
		if (vv_logioerrors && vs->vs_if.if_flags & IFF_DEBUG)
			printf("vv%d: len is zero, vvicsr = %b\n", unit,
			    0xffff&(addr->vvicsr), VV_IBITS);
		goto dropit;
	}
	m = if_rubaget(&vs->vs_ifuba, len, off);
	if (m == NULL) {
		if (vv_logioerrors && vs->vs_if.if_flags & IFF_DEBUG)
			printf("vv%d: if_rubaget failed, vvicsr = %b\n", unit,
			    0xffff&(addr->vvicsr), VV_IBITS);
		goto dropit;
	}
	if (off) {
		m->m_off += 2 * sizeof(u_short);
		m->m_len -= 2 * sizeof(u_short);
	}

	/* Keep track of source address of this packet */
	vs->vs_lastr = vv->vh_shost;
	/*
	 * Demultiplex on packet type
	 */
	switch (vv->vh_type) {

#ifdef INET
	case RING_IP:
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;
#endif
	default:
		printf("vv%d: unknown pkt type 0x%x\n", unit, vv->vh_type);
		m_freem(m);
		goto setup;
	}
	s = splimp();
	if (IF_QFULL(inq)) {
		IF_DROP(inq);
		m_freem(m);
	} else
		IF_ENQUEUE(inq, m);

	splx(s);
	/*
	 * Reset for the next packet.
	 */
setup:
#ifdef BSD2_10
	restorseg5(save5);
#endif BSD2_10
	ubainfo = vs->vs_ifuba.ifu_r.ifrw_info;
	addr->vviba = (u_short) ubainfo;
	addr->vviea = (u_short) (ubainfo >> 16);
	addr->vviwc = -(sizeof (struct vv_header) + VVMTU) >> 1;
	addr->vvicsr = VV_RST | VV_CONF;
	addr->vvicsr |= VV_IEN | VV_DEN | VV_ENB;
	return;

	/*
	 * Drop packet on floor -- count them!!
	 */
dropit:
	vs->vs_if.if_ierrors++;
	goto setup;
}

/*
 * V2lni output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
vvoutput(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	register struct mbuf *m;
	register struct vv_header *vv;
	register int off;
	register int unit;
	register struct vvreg *addr;
	register struct vv_softc *vs;
	register int s;
	int type, error;

#ifdef BSD2_10
	u_long dest;
	segm save5;
	saveseg5(save5);
#else !BSD2_10
	int dest;
#endif BSD2_10

	m = m0;
	unit = ifp->if_unit;
	addr = (struct vvreg *)vvinfo[unit]->ui_addr;
	vs = &vv_softc[unit];
	/*
	 * Check to see if the input side has wedged.
	 *
	 * We are lower than device ipl when we enter this routine,
	 * so if the interface is ready with an input packet then
	 * an input interrupt must have slipped through the cracks.
	 *
	 * Avoid the race with an input interrupt by watching to see
	 * if any packets come in.
	 */
	s = vs->vs_if.if_ipackets;
	if (addr->vvicsr & VV_RDY && s == vs->vs_if.if_ipackets) {
		if (vs->vs_if.if_flags & IFF_DEBUG)
			printf("vv%d: lost a receive interrupt, icsr = %b\n",
			    unit, 0xffff&(addr->vvicsr), VV_IBITS);
		s = splimp();
		vvrint(unit);
		splx(s);
	}

	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		dest = ((struct sockaddr_in *)dst)->sin_addr.s_addr;
#ifdef BSD2_10
		dest = ntohl(in_lnaof(*((struct in_addr *)&dest)));
		if (dest >= 0x100) {
#else !BSD2_10
		if ((dest = in_lnaof(*((struct in_addr *)&dest))) >= 0x100) {
#endif BSD2_10
			error = EPERM;
			goto bad;
		}
		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;
		/* Need per host negotiation. */
		if ((ifp->if_flags & IFF_NOTRAILERS) == 0)
		if (off > 0 && (off & 0x1ff) == 0 &&
		    m->m_off >= MMINOFF + 2 * sizeof (u_short)) {
			type = RING_IPTrailer + (off>>9);
			m->m_off -= 2 * sizeof (u_short);
			m->m_len += 2 * sizeof (u_short);
			*mtod(m, u_short *) = RING_IP;
			*(mtod(m, u_short *) + 1) = m->m_len;
			goto gottrailertype;
		}
		type = RING_IP;
		off = 0;
		goto gottype;
#endif
	default:
		printf("vv%d: can't handle af%d\n", unit, dst->sa_family);
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
#ifdef BSD2_10
	restorseg5(save5);
#endif BSD2_10
	/*
	 * Add local net header.  If no space in first mbuf,
	 * allocate another.
	 */
	if (m->m_off > MMAXOFF ||
	    MMINOFF + sizeof (struct vv_header) > m->m_off) {
#ifdef BSD2_10
		m = m_get(M_DONTWAIT);
#else !BSD2_10
		m = m_get(M_DONTWAIT, MT_HEADER);
#endif BSD2_10
		if (m == NULL) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct vv_header);
	} else {
		m->m_off -= sizeof (struct vv_header);
		m->m_len += sizeof (struct vv_header);
	}
	vv = mtod(m, struct vv_header *);
	vv->vh_shost = ifp->if_host[0];
	/* Map the destination address if it's a broadcast */
	if ((vv->vh_dhost = dest) == INADDR_ANY)
		vv->vh_dhost = VV_BROADCAST;
	vv->vh_version = RING_VERSION;
	vv->vh_type = type;
	vv->vh_info = off;
	vvtracehdr("vo", vv);
#ifdef BSD2_10
	restorseg5(save5);
#endif BSD2_10

	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	s = splimp();
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		error = ENOBUFS;
		goto qfull;
	}
	IF_ENQUEUE(&ifp->if_snd, m);
	if (vs->vs_oactive == 0)
		vvstart(unit);
	splx(s);
	return (0);
qfull:
	m0 = m;
	splx(s);
bad:
	m_freem(m0);
	return(error);
}

/*
 * Process an ioctl request.
 */
vvioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	u_int cmd;
	caddr_t data;
{
	register struct ifreq *ifr;
	register int s;
	int error;

	ifr = (struct ifreq *)data;
	error = 0;
	s = splimp();
	switch (cmd) {

	case SIOCSIFADDR:
		if (ifp->if_flags & IFF_RUNNING)
			if_rtinit(ifp, -1);	/* delete previous route */
		vvsetaddr(ifp, (struct sockaddr_in *)&ifr->ifr_addr);
		if (ifp->if_flags & IFF_RUNNING)
			if_rtinit(ifp, RTF_UP);
		else
			vvinit(ifp->if_unit);
		break;

	default:
		error = EINVAL;
	}
	splx(s);
	return(error);
}

/*
 * Set up the address for this interface. We use the network number
 * from the passed address and an invalid host number; vvinit() will
 * figure out the host number and insert it later.
 */
vvsetaddr(ifp, sin)
	register struct ifnet *ifp;
	register struct sockaddr_in *sin;
{
	ifp->if_net = in_netof(sin->sin_addr);
	ifp->if_host[0] = 256;			/* an invalid host number */
	sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_family = AF_INET;
	sin->sin_addr = if_makeaddr(ifp->if_net, ifp->if_host[0]);
	sin = (struct sockaddr_in *)&ifp->if_broadaddr;
	sin->sin_family = AF_INET;
	sin->sin_addr = if_makeaddr(ifp->if_net, INADDR_ANY);
	ifp->if_flags |= IFF_BROADCAST;
}

/*
 * vvprt_hdr(s, v) print the local net header in "v"
 *	with title is "s"
 */
vvprt_hdr(s, v)
	char *s;
	register struct vv_header *v;
{
	printf("%s: dsvti: 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		s,
		0xff & (int)(v->vh_dhost), 0xff & (int)(v->vh_shost),
		0xff & (int)(v->vh_version), 0xff & (int)(v->vh_type),
		0xffff & (int)(v->vh_info));
}

#ifdef notdef
/*
 * print "l" hex bytes starting at "s"
 */
vvprt_hex(s, l)
	char *s;
	int l;
{
	register int i;
	register int z;

	for (i=0 ; i < l; i++) {
		z = 0xff & (int)(*(s + i));
		printf("%c%c ",
		"0123456789abcdef"[(z >> 4) & 0x0f],
		"0123456789abcdef"[z & 0x0f]
		);
	}
}
#endif notdef
#endif NVV
#endif AUTOCONFIG
