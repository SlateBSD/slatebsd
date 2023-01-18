
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/
/*
 * SCCSID: @(#)if_qe.h	2.4	2/20/86
 */

/* Header files and definitions to support multiple DEQNAs */

#include "socket.h"
#include "../net/if.h"
#include "../net/estat.h"
#include "../net/netisr.h"
#include "../net/route.h"
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/in_var.h"
#include "../netinet/ip.h"
#include "../netinet/ip_var.h"
#include "../netinet/if_ether.h"

#include "if_qereg.h"


/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * is_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
#define	is_if	is_ac.ac_if		/* network-visible interface 	*/
#define	is_addr	is_ac.ac_enaddr		/* hardware Ethernet address 	*/
/* We allocate enough descriptors to do scatter/gather plus 1 to
 * construct a ring.
 *
 * The MicroVAX-I doesn't have an I/O map, therefore all addresses presented
 * to devices must be physical address. To keep track of mbufs in use the
 * softc for this device has to record the mbufs because  the data addresses
 * in the ring descriptors are physical addresses instead of virtual. With
 * an I/O map this will no longer be necessary and we'll be able to use
 * the mbuf macro's for allocation and deallocation.
 *
 * There must be enough descriptors in the receive ring to map at least 2
 * of the largest size datagrams so that a race condition doesn't occur in
 * the receiver. (~1600/252 bytes)
 *
 */
#define NRECV	20	 		/* Receive descriptors		*/
#define NXMIT	15	 		/* Transmit descriptors		*/


struct	qe_softc {
	struct	arpcom is_ac;		/* Ethernet common part 	*/
	struct	qe_ring rring[NRECV+1];	/* Receive ring descriptors 	*/
	short	qe_rzone[2];		/* DEQNA overwrite protection	*/
	struct  mbuf *rmbuf[NRECV+1];	/* Receive mbuf chains		*/
	struct	qe_ring tring[NXMIT+1];	/* Transmit ring descriptors 	*/
	short	qe_tzone[2];		/* DEQNA overwrite protection	*/
	struct  mbuf *tmbuf[NXMIT+1];	/* Transmit mbuf chains		*/
	struct	estat ctrblk;
	long	ztime;			/* Time counters last zeroed	*/
	int	rindex;			/* Receive index		*/
	int	tindex;			/* Transmit index		*/
	int	otindex;		/* Old transmit index		*/
	int	qe_intvec;		/* Interrupt vector 		*/
	struct	qedevice *addr;		/* device addr			*/
	int	timeout;		/* timeout in progress		*/
	int	nxmit;			/* number of transmits		*/
} ;
