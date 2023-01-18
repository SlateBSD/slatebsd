/*	proto.c	4.22	82/06/20	*/

#include "param.h"
#ifdef UCB_NET
#include "../machine/seg.h"
#include "systm.h"
#include "mbuf.h"
#include "domain.h"
#include "protosw.h"
#include "socket.h"
#include "time.h"
#include "kernel.h"

/*
 * Definitions of protocols supported in the UNIX domain.
 */

int	uipc_usrreq();
int	raw_init(),raw_usrreq(),raw_input(),raw_ctlinput();
extern	struct domain unixdomain;		/* or at least forward */

struct protosw unixsw[] = {
{ SOCK_STREAM,	&unixdomain,	0,	PR_CONNREQUIRED|PR_WANTRCVD|PR_RIGHTS,
  0,		0,		0,		0,
  uipc_usrreq,
  0,		0,		0,		0,
},
{ SOCK_DGRAM,	&unixdomain,	0,		PR_ATOMIC|PR_ADDR|PR_RIGHTS,
  0,		0,		0,		0,
  uipc_usrreq,
  0,		0,		0,		0,
},
{ 0,		0,		0,		0,
  raw_input,	0,		raw_ctlinput,	0,
  raw_usrreq,
  raw_init,	0,		0,		0,
}
};

int	unp_externalize(), unp_dispose();

struct domain unixdomain =
    { AF_UNIX, "unix", 0, unp_externalize, unp_dispose,
      unixsw, &unixsw[sizeof(unixsw)/sizeof(unixsw[0])] };

#define	ADDDOMAIN(x)	{ \
	extern struct domain x/**/domain; \
	x/**/domain.dom_next = domains; \
	domains = &x/**/domain; \
}

domaininit()
{
	register struct domain *dp;
	register struct protosw *pr;

#ifndef lint
	ADDDOMAIN(unix);
#ifdef INET
	ADDDOMAIN(inet);
#endif INET
#ifdef NS
	ADDDOMAIN(ns);
#endif NS
#include "imp.h"
#if NIMP > 0
	ADDDOMAIN(imp);
#endif
#endif lint

	for (dp = domains; dp; dp = dp->dom_next) {
		if (dp->dom_init)
			(*dp->dom_init)();
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_init)
				(*pr->pr_init)();
	}
	null_init();
	pffasttimo();
	pfslowtimo();
}

struct protosw *
pffindtype(family, type)
	int family, type;
{
	register struct domain *dp;
	register struct protosw *pr;

	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	return (0);
found:
	for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
		if (pr->pr_type && pr->pr_type == type)
			return (pr);
	return (0);
}

struct protosw *
pffindproto(family, protocol, type)
	int family, protocol, type;
{
	register struct domain *dp;
	register struct protosw *pr;
	struct protosw *maybe = 0;

	if (family == 0)
		return (0);
	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	return (0);
found:
	for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++) {
		if ((pr->pr_protocol == protocol) && (pr->pr_type == type))
			return (pr);

		if (type == SOCK_RAW && pr->pr_type == SOCK_RAW &&
		    pr->pr_protocol == 0 && maybe == (struct protosw *)0)
			maybe = pr;
	}
	return (maybe);
}

pfctlinput(cmd, sa)
	int cmd;
	struct sockaddr *sa;
{
	register struct domain *dp;
	register struct protosw *pr;

	for (dp = domains; dp; dp = dp->dom_next)
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_ctlinput)
				(*pr->pr_ctlinput)(cmd, sa);
}

pfslowtimo()
{
	register struct domain *dp;
	register struct protosw *pr;

	for (dp = domains; dp; dp = dp->dom_next)
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_slowtimo) {
				MAPSAVE();
				(*pr->pr_slowtimo)();
				MAPREST();
			}
	timeout(pfslowtimo, (caddr_t)0, hz/2);
}

pffasttimo()
{
	register struct domain *dp;
	register struct protosw *pr;

	for (dp = domains; dp; dp = dp->dom_next)
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_fasttimo) {
				MAPSAVE();
				(*pr->pr_fasttimo)();
				MAPREST();
			}
	timeout(pffasttimo, (caddr_t)0, hz/5);
}
#endif	UCB_NET
