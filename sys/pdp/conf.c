/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)conf.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "conf.h"
#include "buf.h"
#include "time.h"
#include "ioctl.h"
#include "resource.h"
#include "proc.h"
#include "clist.h"
#include "tty.h"

int	nulldev();
int	nodev();

#include "rk.h"
#if NRK > 0
int	rkopen(), rkstrategy(), rkread(), rkwrite();
#define	rkclose		nulldev
#else
#define	rkopen		nodev
#define	rkclose		nodev
#define	rkstrategy	nodev
#define	rkread		nodev
#define	rkwrite		nodev
#endif

#include "tm.h"
#if NTM > 0
int	tmopen(), tmclose(), tmread(), tmwrite(), tmioctl(), tmstrategy();
#else
#define	tmopen		nodev
#define	tmclose		nodev
#define	tmread		nodev
#define	tmwrite		nodev
#define	tmioctl		nodev
#define	tmstrategy	nodev
#endif

#include "hk.h"
#if NHK > 0
int	hkopen(), hkstrategy(), hkread(), hkwrite(), hkroot();
#define	hkclose		nulldev
#else
#define	hkopen		nodev
#define	hkclose		nodev
#define	hkroot		nulldev
#define	hkstrategy	nodev
#define	hkread		nodev
#define	hkwrite		nodev
#endif

#include "xp.h"
#if NXPD > 0
int	xpopen(), xpstrategy(), xpread(), xpwrite(), xproot();
#define	xpclose		nulldev
#else
#define	xpopen		nodev
#define	xpclose		nodev
#define	xproot		nulldev
#define	xpstrategy	nodev
#define	xpread		nodev
#define	xpwrite		nodev
#endif

#include "ht.h"
#if NHT > 0
int	htopen(), htclose(), htread(), htwrite(), htstrategy(), htioctl();
#else
#define	htopen		nodev
#define	htclose		nodev
#define	htread		nodev
#define	htwrite		nodev
#define	htioctl		nodev
#define	htstrategy	nodev
#endif

#include "rl.h"
#if NRL > 0
int	rlopen(), rlstrategy(), rlread(), rlwrite(), rlroot();
#define	rlclose		nulldev
#else
#define	rlroot		nulldev
#define	rlopen		nodev
#define	rlclose		nodev
#define	rlstrategy	nodev
#define	rlread		nodev
#define	rlwrite		nodev
#endif

#include "ts.h"
#if NTS > 0
int	tsopen(), tsclose(), tsread(), tswrite(), tsstrategy(), tsioctl();
#else
#define	tsopen		nodev
#define	tsclose		nodev
#define	tsread		nodev
#define	tswrite		nodev
#define	tsioctl		nodev
#define	tsstrategy	nodev
#endif

#include "si.h"
#if NSI > 0
int	siopen(), sistrategy(), siread(), siwrite(), siroot();
#define	siclose		nulldev
#else
#define	siopen		nodev
#define	siclose		nodev
#define	siroot		nulldev
#define	sistrategy	nodev
#define	siread		nodev
#define	siwrite		nodev
#endif

#include "ra.h"
#if NRAC > 0
int	rastrategy(), raread(), rawrite(), raroot(), raopen();
#define	raclose		nulldev
#else
#define	raopen		nodev
#define	raclose		nodev
#define	raroot		nulldev
#define	rastrategy	nodev
#define	raread		nodev
#define	rawrite		nodev
#endif

#include "rx.h"
#if NRX > 0
int	rxopen(), rxstrategy(), rxread(), rxwrite(), rxioctl();
#define	rxclose		nulldev
#else
#define	rxopen		nodev
#define	rxclose		nodev
#define	rxstrategy	nodev
#define	rxread		nodev
#define	rxwrite		nodev
#define	rxioctl		nodev
#endif

#include "ram.h"
#if NRAM > 0
int	ramopen(), ramstrategy();
#define ramclose	nulldev
#else
#define ramopen		nodev
#define ramclose	nodev
#define ramstrategy	nodev
#endif

struct bdevsw	bdevsw[] = {
/* ht = 0 */
	htopen,		htclose,	htstrategy,	nulldev,	B_TAPE,
/* tm = 1 */
	tmopen,		tmclose,	tmstrategy,	nulldev,	B_TAPE,
/* ts = 2 */
	tsopen,		tsclose,	tsstrategy,	nulldev,	B_TAPE,
/* ram = 3 */
	ramopen,	ramclose,	ramstrategy,	nulldev,	0,
/* hk = 4 */
	hkopen,		hkclose,	hkstrategy,	hkroot,		0,
/* ra = 5 */
	raopen,		raclose,	rastrategy,	raroot,		0,
/* rk = 6 */
	rkopen,		rkclose,	rkstrategy,	nulldev,	0,
/* rl = 7 */
	rlopen,		rlclose,	rlstrategy,	rlroot,		0,
/* rx = 8 */
	rxopen,		rxclose,	rxstrategy,	nulldev,	0,
/* si = 9 */
	siopen,		siclose,	sistrategy,	siroot,		0,
/* xp = 10 */
	xpopen,		xpclose,	xpstrategy,	xproot,		0,
};
int	nblkdev = sizeof(bdevsw) / sizeof(bdevsw[0]);

int	cnopen(), cnclose(), cnread(), cnwrite(), cnioctl();
extern struct tty cons[];

#include "lp.h"
#if NLP > 0
int	lpopen(), lpclose(), lpwrite();
#else
#define	lpopen		nodev
#define	lpclose		nodev
#define	lpwrite		nodev
#endif

#include "dh.h"
#if NDH > 0
int	dhopen(), dhclose(), dhread(), dhwrite(), dhioctl(), dhstop();
extern struct tty	dh11[];
#else
#define	dhopen		nodev
#define	dhclose		nodev
#define	dhread		nodev
#define	dhwrite		nodev
#define	dhioctl		nodev
#define	dhstop		nodev
#define	dh11		((struct tty *) NULL)
#endif

#include "dz.h"
#if NDZ > 0
int	dzopen(), dzclose(), dzread(), dzwrite(), dzioctl();
int	dzstop();
extern	struct	tty	dz_tty[];
#else
#define	dzopen		nodev
#define	dzclose		nodev
#define	dzread		nodev
#define	dzwrite		nodev
#define	dzioctl		nodev
#define	dzstop		nodev
#define	dz_tty		((struct tty *) NULL)
#endif

#include "pty.h"
#if NPTY > 0
int	ptsopen(), ptsclose(), ptsread(), ptswrite(), ptsstop();
int	ptcopen(), ptcclose(), ptcread(), ptcwrite(), ptyioctl();
int	ptcselect();
extern	struct tty pt_tty[];
#else
#define	ptsopen		nodev
#define	ptsclose	nodev
#define	ptsread		nodev
#define	ptswrite	nodev
#define	ptsstop		nodev
#define	ptcopen		nodev
#define	ptcclose	nodev
#define	ptcread		nodev
#define	ptcwrite	nodev
#define	ptyioctl	nodev
#define	ptcselect	nodev
#define	pt_tty		((struct tty *)NULL)
#endif

#include "dr.h"
#if NDR > 0
int	dropen(), drclose(), drread(), drwrite(), drioctl();
#else
#define	dropen		nodev
#define	drclose		nodev
#define	drread		nodev
#define	drwrite		nodev
#define	drioctl		nodev
#endif

#include "dhu.h"
#if NDHU > 0
int	dhuopen(), dhuclose(), dhuread(), dhuwrite(), dhuioctl(), dhustop();
extern struct tty	dhu_tty[];
#else
#define	dhuopen		nodev
#define	dhuclose	nodev
#define	dhuread		nodev
#define	dhuwrite	nodev
#define	dhuioctl	nodev
#define	dhustop		nodev
#define	dhu_tty		((struct tty *) NULL)
#endif

int	syopen(),syread(),sywrite(),syioctl(),syselect();

int	mmread(),mmwrite();
#define	mmselect	seltrue

int	ttselect(), seltrue();

struct cdevsw	cdevsw[] = {
/* cn = 0 */
	cnopen,		cnclose,	cnread,		cnwrite,
	cnioctl,	nulldev,	cons,		ttselect,
/* mem = 1 */
	nulldev,	nulldev,	mmread,		mmwrite,
	nodev,		nulldev,	0,		mmselect,
/* dz = 2 */
	dzopen,		dzclose,	dzread,		dzwrite,
	dzioctl,	dzstop,		dz_tty,		ttselect,
/* dh = 3 */
	dhopen,		dhclose,	dhread,		dhwrite,
	dhioctl,	dhstop,		dh11,		ttselect,
/* dhu = 4 */
	dhuopen,	dhuclose,	dhuread,	dhuwrite,
	dhuioctl,	dhustop,	dhu_tty,	ttselect,
/* lp = 5 */
	lpopen,		lpclose,	nodev,		lpwrite,
	nodev,		nulldev,	0,		nodev,
/* ht = 6 */
	htopen,		htclose,	htread,		htwrite,
	htioctl,	nulldev,	0,		seltrue,
/* tm = 7 */
	tmopen,		tmclose,	tmread,		tmwrite,
	tmioctl,	nulldev,	0,		seltrue,
/* ts = 8 */
	tsopen,		tsclose,	tsread,		tswrite,
	tsioctl,	nulldev,	0,		seltrue,
/* tty = 9 */
	syopen,		nulldev,	syread,		sywrite,
	syioctl,	nulldev,	0,		syselect,
/* ptc = 10 */
	ptcopen,	ptcclose,	ptcread,	ptcwrite,
	ptyioctl,	nulldev,	pt_tty,		ptcselect,
/* pts = 11 */
	ptsopen,	ptsclose,	ptsread,	ptswrite,
	ptyioctl,	ptsstop,	pt_tty,		ttselect,
/* dr = 12 */
	dropen,		drclose,	drread,		drwrite,
	drioctl,	nulldev,	0,		seltrue,
/* hk = 13 */
	hkopen,		hkclose,	hkread,		hkwrite,
	nodev,		nulldev,	0,		seltrue,
/* ra = 14 */
	raopen,		raclose,	raread,		rawrite,
	nodev,		nulldev,	0,		seltrue,
/* rk = 15 */
	rkopen,		rkclose,	rkread,		rkwrite,
	nodev,		nulldev,	0,		seltrue,
/* rl = 16 */
	rlopen,		rlclose,	rlread,		rlwrite,
	nodev,		nulldev,	0,		seltrue,
/* rx = 17 */
	rxopen,		rxclose,	rxread,		rxwrite,
	rxioctl,	nulldev,	0,		seltrue,
/* si = 18 */
	siopen,		siclose,	siread,		siwrite,
	nodev,		nulldev,	0,		seltrue,
/* xp = 19 */
	xpopen,		xpclose,	xpread,		xpwrite,
	nodev,		nulldev,	0,		seltrue,
};

int	nchrdev = sizeof(cdevsw) / sizeof(cdevsw[0]);
