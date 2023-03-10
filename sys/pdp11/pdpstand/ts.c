/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ts.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 *	standalone TS11 - 1600 bpi tape driver
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

extern int tapemark;	/* flag to indicate tapemark encountered
			   (see sys.c as to how its used) */

int cmdpkt[5];		/* command packet. extra location is mod 4 alignment */

struct	device
{
	caddr_t	tsdb;		/* TS data buffer and bus address reg */
	int	tssr;		/* TS status register */
};

struct	mespkt
{
	int	mshdr;		/* message packet header word */
	int	mssiz;		/* size of message returned */
	int	msresid;	/* remaining count register */
	int	mstsx0;		/* extended status reg 0 */
	int	mstsx1;		/* extended status reg 1 */
	int	mstsx2;		/* extneded status reg 2 */
	int	mstsx3;		/* extended status reg 3 */
};

struct compkt
{
	int	tscom;		/* command packet command word */
	caddr_t	tsba;		/* memory address */
	int	tsbae;		/* extended address */
	int	tswc;		/* byte count, record count, etc. */
};

struct	chrdat
{
	caddr_t	msbptr;		/* pointer to message buffer */
	int	msbae;		/* hi address (bits 16 & 17) */
	int	msbsiz;		/* size of message buffer */
	int	mschar;		/* characteristics word */
};

caddr_t tsptr;
struct	chrdat	chrbuf;		/* characteristics buffer */
struct	mespkt	mesbuf;		/* message buffer */



#define	TSADDR	((struct device *)0172520)
/*	The following 3 lines are necessary to align com buffer on mod 4 */
#define	t_temp	(&cmdpkt[1])	/* get address of cmdpkt +2 */
#define	t_temp1	((int)t_temp & 0177776)
#define	combuf	((struct compkt *)t_temp1)


	/* bit definitions for command word in ts command packet */

#define	ACK	0100000		/* acknowledge bit */
#define	CVC	040000		/* clear volume check */
#define	OPP	020000		/* opposite. reverse recovery */
#define	SWB	010000		/* swap bytes. for data xfer */
	/* bit definitions for Command mode field during read command */
#define	RNEXT	0		/* read next (forward) */
#define	RPREV	0400		/* read previous (reverse) */
#define	RRPRV	01000		/* reread previous (space rev, read two) */
#define	RRNXT	01400		/* reread next (space fwd, read rev) */
	/* bit definitions for Command mode field during write command */
#define	WNEXT	0		/* Write data next */
#define	WDRTY	01000		/* write data retry , space rev, erase, write data) */
	/* bit definitions for command mode field during position command */
#define	SPCFWD	0		/* space records forward */
#define	SPCREV	0400		/* space records reverse */
#define	SKTPF	01000		/* skip tape marks forward */
#define	SKTPR	01400		/* skip tape marks reverse */
#define	RWIND	02000		/* rewind */

	/* bit definitions for command mode field during format command */
#define	WEOF	0		/* write tape mark */
#define	ERAS	0400		/* erase */
#define	WEOFE	01000		/* write tape mark entry */
	/* bit definitions for command mode field during control command */
#define	MBREAL	0		/* message buffer release */
#define	REWUNL	0400		/* Rewind and unload */
#define	CLEAN	01000		/* clean */
	/* additional definitions */
#define	IEI	0200		/* interrupt enable bit */

	/* command code definitions */
#define	NOP	0
#define	RCOM	01		/* read command */
#define	WCHAR	04		/* write characteristics */
#define	WCOM	05		/* write */
#define	WSUSM	06		/* write subsystem memory */
#define	POSIT	010		/* position command */
#define	FORMT	011		/* Format command */
#define	CONTRL	012		/* Control command */
#define	INIT	013		/* initialize */
#define	GSTAT	017		/* get status immediate */


	/* definition of tssr bits */
#define	SC	0100000		/* special condition */
#define	UPE	040000		/* unibus parity error */
#define	SPE	020000		/* Serial bus parity error */
#define	RMR	010000		/* register modify refused */
#define	NXM	04000		/* non-existent memory */
#define	NBA	02000		/* Need Buffer address */
#define	SSR	0200		/* Sub-System ready */
#define	OFL	0100		/* off-line */

	/* fatal termination class codes */
#define	FTC	030		/* use this as a mask to get codes */
	/* code = 00	see error code byte in tsx3 */
	/* code = 01	I/O seq Crom or main Crom parity error */
	/* code = 10	u-processor Crom parity error,I/O silo parity */
	/*		serial bus parity, or other fatal */
	/* code = 11	A/C low. drive ac low */

	/* termination class codes */
#define	TCC	016		/* mask for termination class codes */
	/* code = 000	normal termination */
	/* code = 001	Attention condition 
	/* code = 010	Tape status alert 
	/* code = 011	Function reject 
	/* code = 100	Recoverable error - tape pos = 1 record down from
			start of function 
	/* code = 101	Recoverable error - tape has not moved
	/* code = 110	Unrecoverable error - tape position lost
	/* code = 111	Fatal controller error - see fatal class bits */

	/* definition of message buffer header word */
#define	MAKC	0100000		/* acknowledge from controller */
#define	MCCF	07400		/* mask for class code field */
	/* class codes are */
	/*	0 = ATTN, on or ofline
		1 = ATTN, microdiagnostic failure
		0 = FAIL, serial bus parity error
		1 = FAIL, WRT LOCK
		2 = FAIL, interlock or non-executable function
		3 = FAIL, microdiagnostic error
	*/

#define	MMSC	037	/* mask for message code field */
	/* message codes are
		020	= end
		021	= FAIL
		022	= ERROR
		023	= Attention

	/* definition of extended status reg 0 bits */
#define	TMK	0100000		/* Tape mark detected */
#define	RLS	040000		/* Record length short */
#define	LET	020000		/* Logical end of Tape */
#define	RLL	010000		/* Record length long */
#define	WLE	04000		/* Write lock error */
#define	NEF	02000		/* Non-executable function */
#define	ILC	01000		/* Illegal command */
#define	ILA	0400		/* Illegal address */
#define	MOT	0200		/* Capistan is moving */
#define	ONL	0100		/* On Line */
#define	IE	040		/* state of interrupt enable bit */
#define	VCK	020		/* Volume Check */
#define	PED	010		/* Phase encoded drive */
#define	WLK	04		/* Write locked */
#define	BOT	02		/* Tape at bot */
#define	EOT	01		/* Tape at eot */

	/* definitions of xstat1 */
#define	DLT	0100000		/* Data late error */
#define	COR	020000		/* Correctable data error */
#define	CRS	010000	/* Crease detected */
#define	TIG	04000		/* Trash in the gap */
#define	DBF	02000		/* Deskew Buffer Fail */
#define	SCK	01000		/* Speed check */
#define	IPR	0200		/* Invalid preamble */
#define	SYN	0100		/* Synch Failure */
#define	IPO	040		/* invalid postamble */
#define	IED	020		/* invalid end data */
#define	POS	010		/* postamble short */
#define	POL	04		/* postamble long */
#define	UNC	02		/* Uncorrectable data error */
#define	MTE	01		/* multi track error */

	/* Definitions of XSTAT2 bits */
#define	OPM	0100000		/* operation in progress (tape moving) */
#define	SIP	040000		/* Silo Parity error */
#define	BPE	020000		/* Serial bus Parity error at drive */
#define	CAF	010000		/* Capstan Acceleration fail */
#define	WCF	02000		/* Write card error */
#define	DTP	0477		/* mask for Dead track bits */

	/*	bit definitions for XSTAT3 */
#define	LMX	0200		/* Limit exceeded (tension arms) */
#define	OPI	0100		/* operation incomplete */
#define	REV	040		/* current operation in reverse */
#define	CRF	020		/* capstan response fail */
#define	DCK	010		/* density check */
#define	NOI	04		/* no tape mark or preamble */
#define	LXS	02		/* limit exceeded manual recovery */
#define	RIB	01		/* Reverse into BOT */


#define	SIO	01
#define SSEEK	02
#define	SINIT	04
#define	SRETRY	010
#define	SCOM	020
#define	SOK	040
#define	SMAST	0100

#define	S_WRITTEN	1

tsopen(io)
register struct iob *io;
{
	register skip;

	tsptr = (caddr_t)((int)&combuf->tscom | (int)segflag);
	combuf->tscom = (ACK|CVC|INIT);
	TSADDR->tsdb = tsptr;
	while ((TSADDR->tssr & SSR) == 0);
	chrbuf.msbptr = (caddr_t)&mesbuf;
	chrbuf.msbae = segflag;
	chrbuf.msbsiz = 016;
	chrbuf.mschar = 0;
	combuf->tscom = (ACK|CVC|WCHAR);
	combuf->tsba = (caddr_t)&chrbuf;
	combuf->tsbae = segflag;
	combuf->tswc = 010;
	TSADDR->tsdb = tsptr;
	while ((TSADDR->tssr & SSR) == 0);
	tsstrategy(io,(RWIND|POSIT));
	skip = io->i_boff;
	while (skip--)
	{
		io->i_cc = 1;
		while (tsstrategy(io, (SPCFWD|POSIT)));
	}
}
tsclose(io)
register struct iob *io;
{
	tsstrategy(io,(RWIND|POSIT));
}
tsstrategy(io, func)
register struct iob *io;
{
	register unit, errcnt;

	unit = io->i_unit;
	errcnt = 0;
	combuf->tsba = io->i_ma;
	combuf->tsbae = segflag;
	combuf->tswc = io->i_cc;
	if (func == READ)
		combuf->tscom = ACK|RNEXT|RCOM;
	else if (func == WRITE)
		combuf->tscom = ACK|WNEXT|WCOM;
	else if (func == SPCREV)
	{
		combuf->tscom = ACK|SPCREV|POSIT;
		combuf->tswc = 1;
	}
	else
		combuf->tscom = ACK|func;
	TSADDR->tsdb = tsptr;
retry:
	while ((TSADDR->tssr & SSR) == 0);
	if (mesbuf.mstsx0 & TMK)
	{
		tapemark = 1;
		return(0);
	}
	if ((mesbuf.mshdr & MMSC) == 021 || (mesbuf.mshdr & MMSC) == 022)
	{
		if(errcnt == 0)
			printf("tape error: er=%o",mesbuf.mstsx0);
		if(errcnt == 10)
		{
			printf("\n");
			return(-1);
		}
		errcnt++;
		if(func == READ)
			combuf->tscom = (ACK|RPREV|RCOM);
		else if (func == WRITE)
			combuf->tscom = (ACK|WDRTY|WCOM);
		else
		{
			printf("\n");
			return(-1);
		}
		TSADDR->tsdb = tsptr;
		goto retry;
	}
	if(errcnt)
		printf(" recovered by retry\n");
	return (io->i_cc+mesbuf.msresid);
}
