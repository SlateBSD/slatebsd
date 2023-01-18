/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)short_names.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

#ifndef _SHORT_NAMES
#define _SHORT_NAMES

#ifdef KERNEL
#define	avefree30		avefr30
#define	copyoutstr		cpyostr
#define	hostnamelen		hostnlen
#define	unp_connect2		unp_cn2		/* uipc_usrreq.c */
#define	unp_discard		unp_dcard	/* uipc_usrreq.c */
#define	unp_dispose		unp_dpose	/* uipc_usrreq.c */
#endif

#define	alloc_cacheflush	alloc_flcache
#define	dom_protosw		dproto		/* domain.h */
#define	dom_protoswNPROTOSW	dprotoLAST	/* domain.h */
#define	free_cacheswap		free_swcache
#define	gethostbyaddr		gethbyaddr
#define	gethostbyname		gethbyname
#define	gethostent		gethent
#define	gethostid		gethstid
#define	gethostname		gethname
#define	getnetbyaddr		getnbyaddr
#define	getnetbyname		getnbyname
#define	getnetent		getnent
#define	getprotobyname		getpbyname
#define	getprotobynumber	getpbnumber
#define	getprotoent		getpent
#define	getservbyname		getsbyname
#define	getservbyport		getsbyport
#define	getservent		getsent
#define	getsockname		getsname
#define	ia_subnetmask		ia_submask
#define	idp_ctlinput		idp_ctinput	/* netns/ns_proto.c */
#define	idp_ctloutput		idp_ctoutput	/* netns/ns_proto.c */
#define	idps_badhlen		idps_bhlen	/* idp_var.h */
#define	idps_badlen		idps_blen	/* idp_var.h */
#define	idps_tooshort		idps_tshort	/* idp_var.h */
#define	idps_toosmall		idps_tsmall	/* idp_var.h */
#define	if_ifwithaddr		if_iwaddr	/* XXX delete? */
#define	if_ifwithaf		if_iwaf		/* XXX delete? */
#define	if_ifwithnet		if_iwnet	/* XXX delete? */
#define	ifa_ifwithaddr		ifa_waddr
#define	ifa_ifwithaf		ifa_waf
#define	ifa_ifwithdstaddr	ifa_wdsta
#define	ifa_ifwithnet		ifa_wnet
#define	imphostdead		imphdead	/* etc/implog */
#define	imphostunreach		imphunreach	/* etc/implog */
#define	in_pcballoc		in_pcbg
#define	in_pcbdetach		in_pcbetach
#define	inet_netof		inet_nof
#define	inet_network		inet_nwk
#define	int_subnetmask		sbntmsk
#define	ipforward_rt		ip_fwdrt
#define	msg_accrights		msg_acr
#define	msg_accrightslen	msg_acrl
#define	msg_iov			msgiov
#define	msg_iovlen		msgiovl
#define	msg_name		msg_nm
#define	msg_namelen		msg_nml
#define	ns_broadhost		ns_brhost	/* netns/ns.h */
#define	ns_broadnet		ns_brnet	/* netns/ns.h */
#define	ns_es_badcode		ns_es_bcode	/* ns_error.h */
#define	ns_es_badlen		ns_es_blen	/* ns_error.h */
#define	ns_es_oldns_err		ns_es_oerr	/* ns_error.h */
#define	ns_es_oldshort		ns_es_oshort	/* ns_error.h */
#define	ns_output_cnt		ns_outcnt	/* ns_output.c */
#define	ns_zerohost		ns_hzero	/* ns.h */
#define	ns_zeronet		ns_nzero	/* ns.h */
#define	protoswLAST		protoLAST
#define	rawintr			rawint
#define	sbappendaddr		sbappadd
#define	sbappendrecord		sbapprecord	/* socketsubr.c */
#define	sbappendrights		sbapprights	/* socketsubr.c */
#define	sethostent		sethent
#define	sethostfile		sethfile
#define	sethostid		sethstid
#define	sethostname		sethname
#define	sockaddr_in		sock_in
#define	sockaddr_pup		sock_pup	/* XXX delete? */
#define	sockaddr_un		sock_un
#define	soconnect2		soconn2
#define	soisconnected		soisced
#define	soisconnecting		soiscing
#define	soisdisconnected	soisded
#define	soisdisconnecting	soisding
#define	spp_ctlinput		spp_ictl	/* netns/ns_proto.c */
#define	spp_ctloutput		spp_octl	/* netns/ns_proto.c */
#define	spp_debug		spp_dbug	/* netns/ns_debug.h */
#define	spp_output_cnt		spp_ocnt	/* netns/spp_usrreq.c */
#define	spp_usrclosed		spp_uclosed	/* netns/spp_usrreq.c */
#define	spp_usrreq_sp		spp_ursp	/* netns/ns_proto.c */
#define	spup_zero1		spup_z1		/* XXX delete? */
#define	spup_zero2		spup_z2		/* XXX delete? */
#define	ssocketaddr		ssockad		/* XXX delete? */
#define	tcp_ctlinput		tcp_ictl	/* in_proto.c */
#define	tcp_ctloutput		tcp_octl	/* in_proto.c */
#define	tcp_debx		tcp_dbx
#define	tcp_initopt		tcp_iopt
#define	tcp_output		tcp_oput
#define	tcp_usrclosed		tcp_uclosed
#define	tcp_usrreq		tcp_ureq	/* tcp_usrreq.c */
#define	tcps_badoff		tcps_boff
#define	tcps_badsegs		tcps_bsegs
#define	tcps_badsum		tcps_bsum
#define	tcpstates		tcpstas
#define	udps_badlen		udps_blen
#define	udps_badsum		udps_bsum
#define	ns_pcbdetach		ns_pcbetach	/* ns_pcb.c */
#define	nsintr_getpck		nsi_getpck	/* ns_input.c */
#define	nsintr_swtch		nsi_swtch	/* ns_input.c */
#define	sockaddr_ns		sock_ns
#endif !_SHORT_NAMES
