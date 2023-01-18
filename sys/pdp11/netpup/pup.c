/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)pup.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "npup.h"
#if NPUP > 0

#include "param.h"
#include "mbuf.h"
#include "protosw.h"
#include "socket.h"
#include "socketvar.h"
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/af.h>
#include <netpup/pup.h>

pup_hash(spup, hp)
	struct sockaddr_pup *spup;
	struct afhash *hp;
{
	hp->afh_nethash = spup->spup_addr.pp_net;
	hp->afh_hosthash = spup->spup_addr.pp_host;
	if (hp->afh_hosthash < 0)
		hp->afh_hosthash = -hp->afh_hosthash;
}

pup_netmatch(spup1, spup2)
	struct sockaddr_pup *spup1, *spup2;
{
	return (spup1->spup_addr.pp_net == spup2->spup_addr.pp_net);
}
#endif
