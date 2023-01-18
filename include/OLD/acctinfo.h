/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)acctinfo.h	5.1 (Berkeley) 12/13/86
 */

struct account {
	int	ac_acct;	/* whom to charge	*/
	int	ac_pages;	/* lpr pages		*/
	int	ac_quota;	/* maximum quota	*/
	long	ac_sys;		/* system cpu		*/
	long	ac_usr;		/* usr cpu		*/
	long	ac_real;	/* real time use	*/
	long	ac_ttyio;	/* tty i/o		*/
	long	ac_diskio;	/* disk i/o		*/
	long	ac_m1;		/* miscellaneous	*/
	long	ac_m2;
	long	ac_m3;
	long	ac_m4;
	long	ac_m5;
};
