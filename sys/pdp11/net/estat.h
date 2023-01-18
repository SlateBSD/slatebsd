/*
 * Taken directly from ULTRIX-11; not included in "net/if.h" deliberately.
 */

/*
 * interface statistics structures
 */

#ifdef	pdp11
#define	dst_localsta		dstlst
#define	dst_localsta_bm		dstlst_b
#define	dst_localbuf		dstlbuf
#define	dst_localbuf_bm		dstlbf_b
#define	dst_localtmo		dstlcltm
#define	dst_remotesta		dstrsta
#define	dst_remotesta_bm	dstrst_b
#define	dst_remotebuf		dstrbuf
#define	dst_remotebuf_bm	dstrbf_b
#define	dst_remotetmo		dstrtmo
#define	dst_selecttmo		dstsltm
#define	dst_selecttmo_bm	dstslt_b
#define	dst_select		dstsl
#define	dst_outbound		dstobnd
#define	dst_outbound_bm		dstobn_b
#define	dst_bytesent		dstbsent
#define	dst_bytercvd		dstbrcvd
#define	est_recvfail		estrfail
#define	est_recvfail_bm		estrfl_b
#define	est_sendfail		estsfail
#define	est_sendfail_bm		estsfl_b
#define	est_bloksent		estblsen
#define	est_blokrcvd		estblrcv
#define	est_bytesent		estbysen
#define	est_bytercvd		estbyrcv
#define	pst_inbound		pstibnd
#define	pst_inbound_bm		pstinb_b
#endif	pdp11

struct estat {				/* Ethernet interface statistics */
	u_short	est_seconds;		/* seconds since last zeroed */
	u_int	est_bytercvd;		/* bytes received */
	u_int	est_bytesent;		/* bytes sent */
	u_int	est_mbytercvd;		/* multicast bytes received */
	u_int	est_blokrcvd;		/* data blocks received */
	u_int	est_bloksent;		/* data blocks sent */
	u_int	est_mblokrcvd;		/* multicast blocks received */
	u_int	est_deferred;		/* blocks sent, initially deferred */
	u_int	est_single;		/* blocks sent, single collision */
	u_int	est_multiple;		/* blocks sent, multiple collisions */
	u_short	est_sendfail_bm;	/*	0 - Excessive collisions */
					/*	1 - Carrier check failed */
					/*	2 - Short circuit */
					/*	3 - Open circuit */
					/*	4 - Frame too long */
					/*	5 - Remote failure to defer */
	u_short	est_sendfail;		/* send failures: (bit map)*/
	u_short	est_collis;		/* Collision detect check failure */
	u_short	est_recvfail_bm;	/*	0 - Block check error */
					/*	1 - Framing error */
					/*	2 - Frame too long */
	u_short	est_recvfail;		/* receive failure: (bit map) */
	u_short	est_unrecog;		/* unrecognized frame destination */
	u_short	est_overrun;		/* data overrun */
	u_short	est_sysbuf;		/* system buffer unavailable */
	u_short	est_userbuf;		/* user buffer unavailable */
};

struct dstat {				/* DDCMP pt-to-pt interface statistics */
	u_short	dst_seconds;		/* seconds since last zeroed */
	u_int	dst_bytercvd;		/* bytes received */
	u_int	dst_bytesent;		/* bytes sent */
	u_int	dst_blokrcvd;		/* data blocks received */
	u_int	dst_blocksent;		/* data blocks sent */
	u_short	pst_inbound_bm;		/*	0 - NAKs sent, header crc */
					/*	1 - NAKs sent, data crc */
					/*	2 - NAKs sent, REP response */
	u_char	pst_inbound;		/* data errors inbound: (bit map) */
	u_short	dst_outbound_bm;	/*	0 - NAKs rcvd, header crc */
					/*	1 - NAKs rcvd, data crc */
					/*	2 - NAKs rcvd, REP response */
	u_char	dst_outbound;		/* data errors outbound: (bit map) */
	u_char	dst_remotetmo;		/* remote reply timeouts */
	u_char	dst_localtmo;		/* local reply timeouts */
	u_short	dst_remotebuf_bm;	/*	0 - NAKs rcvd, buffer unavailable */
					/*	1 - NAKs rcvd, buffer too small */
	u_char	dst_remotebuf;		/* remote buffer errors: (bit map) */
	u_short	dst_localbuf_bm;	/*	0 - NAKs sent, buffer unavailable */
					/*	1 - NAKs sent, buffer too small */
	u_char	dst_localbuf;		/* local buffer errors: (bit map) */
	u_char	dst_select;		/* selection intervals elapsed */
	u_short	dst_selecttmo_bm;	/*	0 - No reply to select */
					/*	1 - Incomplete reply to select */
	u_char	dst_selecttmo;		/* selection timeouts: (bit map) */
	u_short	dst_remotesta_bm;	/*	0 - NAKs rcvd, receive overrun */
					/*	1 - NAKs sent, header format */
					/*	2 - Select address errors
					/*	3 - Streaming tributaries */
	u_char	dst_remotesta;		/* remote station errors: (bit map) */
	u_short	dst_localsta_bm;	/*	0 - NAKs sent, receive overrun */
					/*	1 - Receive overrun, NAK not sent */
					/*	2 - Transmit underruns */
					/*	3 - NAKs rcvd, header format */
	u_char	dst_localsta;		/* local station errors: (bit map) */
};
