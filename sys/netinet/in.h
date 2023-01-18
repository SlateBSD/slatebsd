/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)in.h	7.1 (Berkeley) 6/5/86
 */

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981.
 */

#ifdef BSD2_10
#include <short_names.h>
#endif

/*
 * Protocols
 */
#define	IPPROTO_IP		0		/* dummy for IP */
#define	IPPROTO_ICMP		1		/* control message protocol */
#define	IPPROTO_GGP		2		/* gateway^2 (deprecated) */
#define	IPPROTO_TCP		6		/* tcp */
#define	IPPROTO_EGP		8		/* exterior gateway protocol */
#define	IPPROTO_PUP		12		/* pup */
#define	IPPROTO_UDP		17		/* user datagram protocol */
#define	IPPROTO_IDP		22		/* xns idp */

#define	IPPROTO_RAW		255		/* raw IP packet */
#define	IPPROTO_MAX		256


/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.
 */
#define	IPPORT_RESERVED		1024
#define	IPPORT_USERRESERVED	5000

/*
 * Link numbers
 */
#define	IMPLINK_IP		155
#define	IMPLINK_LOWEXPER	156
#define	IMPLINK_HIGHEXPER	158

/*
 * Internet address (a structure for historical reasons)
 */
struct in_addr {
	u_long s_addr;
};

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define	IN_CLASSA(i)		(((long)(i) & 0x80000000L) == 0)
#define	IN_CLASSA_NET		0xff000000L
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		0x00ffffffL
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(i)		(((long)(i) & 0xc0000000L) == 0x80000000L)
#define	IN_CLASSB_NET		0xffff0000L
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		0x0000ffffL
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(i)		(((long)(i) & 0xc0000000L) == 0xc0000000L)
#define	IN_CLASSC_NET		0xffffff00L
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		0x000000ffL

#define	IN_CLASSD(i)		(((long)(i) & 0xf0000000L) == 0xe0000000L)
#define	IN_MULTICAST(i)		IN_CLASSD(i)

#define	IN_EXPERIMENTAL(i)	(((long)(i) & 0xe0000000L) == 0xe0000000L)
#define	IN_BADCLASS(i)		(((long)(i) & 0xf0000000L) == 0xf0000000L)

#define	INADDR_ANY		0x00000000L
#define	INADDR_BROADCAST	0xffffffffL	/* must be masked */
#ifndef KERNEL
#define	INADDR_NONE		0xffffffffL	/* -1 return */
#endif

#define	IN_LOOPBACKNET		127		/* official! */

#ifdef BSD2_10			/* used in lib/libc/inet/inet_netof.c */
#define	IN_CLASSA_NET_SHIFTMASK	0x000000ffL
#define	IN_CLASSB_NET_SHIFTMASK	0x0000ffffL
#define	IN_CLASSC_NET_SHIFTMASK	0x00ffffffL
#endif

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
	short	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};

/*
 * Options for use with [gs]etsockopt at the IP level.
 */
#define	IP_OPTIONS	1		/* set/get IP per-packet options */

#ifdef KERNEL
extern	struct domain inetdomain;
extern	struct protosw inetsw[];
struct	in_addr in_makeaddr();
u_long	in_netof(), in_lnaof();
#endif
