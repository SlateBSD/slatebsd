/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)tcp_subr.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "systm.h"
#include "mbuf.h"
#include "domain.h"
#include "protosw.h"
#include "socket.h"
#include "socketvar.h"
#include "errno.h"

#include "../net/route.h"
#include "../net/if.h"

#include "in.h"
#include "in_pcb.h"
#include "in_systm.h"
#include "ip.h"
#include "ip_var.h"
#include "ip_icmp.h"
#include "tcp.h"
#include "tcp_fsm.h"
#include "tcp_seq.h"
#include "tcp_timer.h"
#include "tcp_var.h"
#include "tcpip.h"

int	tcp_ttl = TCP_TTL;

/*
 * Tcp initialization
 */
tcp_init()
{

	tcp_iss = 1;		/* wrong */
	tcb.inp_next = tcb.inp_prev = &tcb;
	tcp_alpha = TCP_ALPHA;
	tcp_beta = TCP_BETA;
}

/*
 * Create template to be used to send tcp packets on a connection.
 * Call after host entry created, allocates an mbuf and fills
 * in a skeletal tcp/ip header, minimizing the amount of work
 * necessary when the connection is used.
 */
struct tcpiphdr *
tcp_template(tp)
	struct tcpcb *tp;
{
	register struct inpcb *inp = tp->t_inpcb;
	register struct tcpiphdr *n;

	MSGET(n, struct tcpiphdr, M_CLEAR);
	if (n == NULL)
		return ((struct tcpiphdr *)0);
	n->ti_next = n->ti_prev = 0;
	n->ti_pad = 0;
	n->ti_x1 = 0;
	n->ti_pr = IPPROTO_TCP;
	n->ti_len = htons(sizeof (struct tcpiphdr) - sizeof (struct ip));
	n->ti_src = inp->inp_laddr;
	n->ti_dst = inp->inp_faddr;
	n->ti_sport = inp->inp_lport;
	n->ti_dport = inp->inp_fport;
	n->ti_seq = 0;
	n->ti_ack = 0;
	n->ti_x2 = 0;
	n->ti_off = 5;
	n->ti_flags = 0;
	n->ti_win = 0;
	n->ti_sum = 0;
	n->ti_urp = 0;
	return (n);
}

/*
 * Send a single message to the TCP at address specified by
 * the given TCP/IP header.  If flags==0, then we make a copy
 * of the tcpiphdr at ti and send directly to the addressed host.
 * This is used to force keep alive messages out using the TCP
 * template for a connection tp->t_template.  If flags are given
 * then we send a message back to the TCP which originated the
 * segment ti, and discard the mbuf containing it and any other
 * attached mbufs.
 *
 * In any case the ack and sequence number of the transmitted
 * segment are as specified by the parameters.
 */
tcp_respond(tp, ti, ack, seq, flags)
	struct tcpcb *tp;
	register struct tcpiphdr *ti;
	tcp_seq ack, seq;
	int flags;
{
	register struct mbuf *m;
	int win = 0, tlen;
	struct route *ro = 0;

	MAPSAVE();
	if (tp) {
		win = sbspace(&tp->t_inpcb->inp_socket->so_rcv);
		ro = &tp->t_inpcb->inp_route;
	}
	if (flags == 0) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == NULL) {
			MAPUNSAVE();
			return;
		}
		m->m_len = sizeof (struct tcpiphdr) + 1;
/* THIS WON'T WORK!!! (ti is in an mbuf) */
		bcopy((caddr_t)ti, mtod(m, caddr_t), sizeof *ti);
		ti = mtod(m, struct tcpiphdr *);
		flags = TH_ACK;
#ifdef TCP_COMPAT_42
		tlen = 1;
#else
		tlen = 0;
#endif
	} else {
		m = dtom(ti);
		m_freem(m->m_next);
		m->m_next = 0;
#ifdef BSD2_10
		m->m_off = (caddr_t)ti - (caddr_t)MBX;
#else
		m->m_off = (int)ti - (int)m;
#endif
		m->m_len = sizeof (struct tcpiphdr);
#define xchg(a,b,type) { type t; t=a; a=b; b=t; }
		xchg(ti->ti_dst.s_addr, ti->ti_src.s_addr, u_long);
		xchg(ti->ti_dport, ti->ti_sport, u_short);
#undef xchg
		tlen = 0;
	}
	bzero((caddr_t)ti, 9);
	ti->ti_len = htons((u_short)(sizeof (struct tcphdr) + tlen));
	ti->ti_seq = htonl(seq);
	ti->ti_ack = htonl(ack);
	ti->ti_x2 = 0;
	ti->ti_off = sizeof (struct tcphdr) >> 2;
	ti->ti_flags = flags;
	ti->ti_win = htons((u_short)win);
	ti->ti_urp = 0;
	ti->ti_sum = in_cksum(m, sizeof (struct tcpiphdr) + tlen);
	((struct ip *)ti)->ip_len = sizeof (struct tcpiphdr) + tlen;
	((struct ip *)ti)->ip_ttl = TCP_TTL;
	MAPREST();
	(void) ip_output(m, (struct mbuf *)0, ro, 0);
}

/*
 * Create a new TCP control block, making an
 * empty reassembly queue and hooking it to the argument
 * protocol control block.
 */
struct tcpcb *
tcp_newtcpcb(inp)
	struct inpcb *inp;
{
	register struct tcpcb *tp;

	MSGET(tp, struct tcpcb, M_CLEAR);
	if (tp == NULL)
		return ((struct tcpcb *)0);
	tp->seg_next = tp->seg_prev = (struct tcpiphdr *)tp;
	tp->t_maxseg = TCP_MSS;
	tp->t_flags = 0;		/* sends options! */
	tp->t_inpcb = inp;
	tp->t_srtt = TCPTV_SRTTBASE;
	tp->snd_cwnd = sbspace(&inp->inp_socket->so_snd);
	inp->inp_ppcb = (caddr_t)tp;
	return (tp);
}

/*
 * Drop a TCP connection, reporting
 * the specified error.  If connection is synchronized,
 * then send a RST to peer.
 */
struct tcpcb *
tcp_drop(tp, errno)
	register struct tcpcb *tp;
	int errno;
{
	struct socket *so = tp->t_inpcb->inp_socket;

	if (TCPS_HAVERCVDSYN(tp->t_state)) {
		tp->t_state = TCPS_CLOSED;
		(void) tcp_output(tp);
	}
	so->so_error = errno;
	return (tcp_close(tp));
}

#ifdef BSD2_10
#define ti_mbuf ti_sum
#define DTOM(d) ( (struct mbuf *) ((d)->ti_mbuf) )
#else
#define DTOM(d) dtom(d)
#endif

/*
 * Close a TCP control block:
 *	discard all space held by the tcp
 *	discard internet protocol block
 *	wake up any sleepers
 */
struct tcpcb *
tcp_close(tp)
	register struct tcpcb *tp;
{
	register struct tcpiphdr *t,*to;
	struct inpcb *inp = tp->t_inpcb;
	struct socket *so = inp->inp_socket;

	t = tp->seg_next;
	while (t != (struct tcpiphdr *)tp) {
		m_freem(DTOM(t));
		to = t;
		remque(to);
		t = (struct tcpiphdr *)t->ti_next;
#ifdef BSD2_10
		MSFREE(to);
#endif
	}
	if (tp->t_template)
		(void) MSFREE(tp->t_template);
	if (tp->t_tcpopt)
		(void) m_free(tp->t_tcpopt);
	(void) MSFREE(tp);
	inp->inp_ppcb = 0;
	soisdisconnected(so);
	in_pcbdetach(inp);
	return ((struct tcpcb *)0);
}

tcp_drain()
{

}

#ifdef notdef
/*
 * Notify a tcp user of an asynchronous error;
 * just wake up so that he can collect error status.
 */
tcp_notify(inp)
	register struct inpcb *inp;
{

	wakeup((caddr_t) &inp->inp_socket->so_timeo);
	sorwakeup(inp->inp_socket);
	sowwakeup(inp->inp_socket);
}
#endif
tcp_ctlinput(cmd, sa)
	int cmd;
	struct sockaddr *sa;
{
	extern u_char inetctlerrmap[];
	struct sockaddr_in *sin;
	int tcp_quench(), in_rtchange();

	if ((unsigned)cmd > PRC_NCMDS)
		return;
	if (sa->sa_family != AF_INET && sa->sa_family != AF_IMPLINK)
		return;
	sin = (struct sockaddr_in *)sa;
	if (sin->sin_addr.s_addr == INADDR_ANY)
		return;

	switch (cmd) {

	case PRC_QUENCH:
		in_pcbnotify(&tcb, &sin->sin_addr, 0, tcp_quench);
		break;

	case PRC_ROUTEDEAD:
	case PRC_REDIRECT_NET:
	case PRC_REDIRECT_HOST:
	case PRC_REDIRECT_TOSNET:
	case PRC_REDIRECT_TOSHOST:
		in_pcbnotify(&tcb, &sin->sin_addr, 0, in_rtchange);
		break;

	default:
		if (inetctlerrmap[cmd] == 0)
			return;		/* XXX */
		in_pcbnotify(&tcb, &sin->sin_addr, (int)inetctlerrmap[cmd],
			(int (*)())0);
	}
}

/*
 * When a source quench is received, close congestion window
 * to 80% of the outstanding data (but not less than one segment).
 */
tcp_quench(inp)
	struct inpcb *inp;
{
	struct tcpcb *tp = intotcpcb(inp);

	if (tp)
	    tp->snd_cwnd = MAX(8 * ((tp->snd_nxt - tp->snd_una) / 10),
		tp->t_maxseg);
}
