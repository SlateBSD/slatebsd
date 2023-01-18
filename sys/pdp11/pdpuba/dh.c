/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)dh.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * DH11, DM11 device drivers
 */

#include "dh.h"
#if NDH > 0
/*
 * DH-11/DM-11 driver
 */
#include "param.h"
#include "conf.h"
#include "user.h"
#include "file.h"
#include "ioctl.h"
#include "tty.h"
#include "dhreg.h"
#include "dmreg.h"
#include "map.h"
#include "uba.h"
#include "ubavar.h"
#include "clist.h"
#include "systm.h"
#include "vm.h"
#include "kernel.h"
#include "proc.h"

int	dhtimer();
struct	uba_device dhinfo[NDH];
struct	uba_device dminfo[NDH];

#ifndef	PORTSELECTOR
#define	ISPEED	B9600
#define	IFLAGS	(EVENP|ODDP|ECHO)
#else
#define	ISPEED	B4800
#define	IFLAGS	(EVENP|ODDP)
#endif

#define	FASTTIMER	(LINEHZ/30)	/* scan rate with silos on */

/*
 * Local variables for the driver
 */
short	dhsar[NDH];			/* software copy of last bar */
short	dhsoftCAR[NDH];

struct	tty dh11[NDH*16];
int	ndh11	= NDH*16;
int	dhact;				/* mask of active dh's */
int	dhsilos;			/* mask of dh's with silo in use */
int	dhchars[NDH];			/* recent input count */
int	dhrate[NDH];			/* smoothed input count */
int	dhhighrate = 100;		/* silo on if dhchars > dhhighrate */
int	dhlowrate = 75;			/* silo off if dhrate < dhlowrate */
static short timerstarted;
int	dhstart(), ttrstrt();

#if defined(UNIBUS_MAP) || defined(UCB_CLIST)
extern	ubadr_t	clstaddr;
#define	cpaddr(x)	(clstaddr + (ubadr_t)((x) - (char *)cfree))
#else
#define	cpaddr(x)	(x)
#endif

#define	UNIT(x)	(minor(x) & 0177)

/*
 * Routine called to attach a dh.
 */
dhattach(addr, unit)
	register caddr_t addr;
	register u_int unit;
{
	register struct uba_device *ui;

	if (addr && unit < NDH && !dhinfo[unit].ui_addr) {
		ui = &dhinfo[unit];
		ui->ui_unit = unit;
		ui->ui_addr = addr;
		ui->ui_alive = 1;
		return (1);
	}
	return (0);
}

/*ARGSUSED*/
dmattach(addr, unit)
	register caddr_t addr;
	register u_int unit;
{
	register struct uba_device *ui;

	if (addr && unit < NDH && !dminfo[unit].ui_addr) {
		ui = &dminfo[unit];
		ui->ui_unit = unit;
		ui->ui_addr = addr;
		ui->ui_alive = 1;
		return (1);
	}
	return (0);
}

/*
 * Open a DH11 line.  Turn on this dh if this is
 * the first use of it.  Also do a dmopen to wait for carrier.
 */
/*ARGSUSED*/
dhopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register int unit, dh;
	register struct dhdevice *addr;
	register struct uba_device *ui;
	int s;

	unit = UNIT(dev);
	dh = unit >> 4;
	if (unit >= NDH*16 || (ui = &dhinfo[dh])->ui_alive == 0)
		return (ENXIO);
	tp = &dh11[unit];
	if (tp->t_state&TS_XCLUDE && u.u_uid!=0)
		return (EBUSY);
	addr = (struct dhdevice *)ui->ui_addr;
	tp->t_addr = (caddr_t)addr;
	tp->t_oproc = dhstart;
	tp->t_state |= TS_WOPEN;

	/*
	 * While setting up state for this uba and this dh,
	 * block uba resets which can clear the state.
	 */
	s = spl5();
	if (timerstarted == 0) {
		timerstarted++;
		timeout(dhtimer, (caddr_t) 0, LINEHZ);
	}
	if ((dhact&(1<<dh)) == 0) {
		addr->un.dhcsr |= DH_IE;
		dhact |= (1<<dh);
		addr->dhsilo = 0;
	}
	splx(s);
	/*
	 * If this is first open, initialize tty state to default.
	 */
	if ((tp->t_state&TS_ISOPEN) == 0) {
		ttychars(tp);
#ifndef PORTSELECTOR
		if (tp->t_ispeed == 0) {
#else
			tp->t_state |= TS_HUPCLS;
#endif PORTSELECTOR
			tp->t_ispeed = ISPEED;
			tp->t_ospeed = ISPEED;
			tp->t_flags = IFLAGS;
#ifndef PORTSELECTOR
		}
#endif
		dhparam(unit);
	}
	dmopen(dev);
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*
 * Close a DH line, turning off the DM11.
 */
/*ARGSUSED*/
dhclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register unit;

	unit = UNIT(dev);
	tp = &dh11[unit];
	(*linesw[tp->t_line].l_close)(tp);
	((struct dhdevice *)(tp->t_addr))->dhbreak &= ~(1<<(unit&017));
	if (tp->t_state&TS_HUPCLS || (tp->t_state&TS_ISOPEN)==0)
		dmctl(unit, DML_OFF, DMSET);
	ttyclose(tp);
}

dhread(dev)
	dev_t dev;
{
	register struct tty *tp = &dh11[UNIT(dev)];

	return ((*linesw[tp->t_line].l_read)(tp));
}

dhwrite(dev)
	dev_t dev;
{
	register struct tty *tp = &dh11[UNIT(dev)];

	return ((*linesw[tp->t_line].l_write)(tp));
}

/*
 * DH11 receiver interrupt.
 */
dhrint(dh)
	int dh;
{
	register struct tty *tp;
	register int c;
	register struct dhdevice *addr;
	struct tty *tp0;
	struct uba_device *ui;
	int overrun = 0;

	ui = &dhinfo[dh];
	if (ui == 0 || ui->ui_alive == 0)
		return;
	addr = (struct dhdevice *)ui->ui_addr;
	tp0 = &dh11[dh<<4];
	/*
	 * Loop fetching characters from the silo for this
	 * dh until there are no more in the silo.
	 */
	while ((c = addr->dhrcr) < 0) {
		tp = tp0 + ((c>>8)&0xf);
		dhchars[dh]++;
		if ((tp->t_state&TS_ISOPEN)==0) {
			wakeup((caddr_t)&tp->t_rawq);
#ifdef PORTSELECTOR
			if ((tp->t_state&TS_WOPEN) == 0)
#endif
			continue;
		}
		if (c & DH_PE)
			if ((tp->t_flags & (EVENP|ODDP)) == EVENP
			 || (tp->t_flags & (EVENP|ODDP)) == ODDP)
				continue;
		if ((c & DH_DO) && overrun == 0) {
			printf("dh%d: silo overflow\n", dh);
			overrun = 1;
		}
		if (c & DH_FE)
			/*
			 * At framing error (break) generate
			 * a null (in raw mode, for getty), or an
			 * interrupt (in cooked/cbreak mode).
			 */
			if (tp->t_flags & RAW)
				c = 0;
			else
				c = tp->t_intrc;
#if NBK > 0
		if (tp->t_line == NETLDISC) {
			c &= 0177;
			BKINPUT(c, tp);
		} else
#endif
			(*linesw[tp->t_line].l_rint)(c, tp);
	}
}

/*
 * Ioctl for DH11.
 */
/*ARGSUSED*/
dhioctl(dev, cmd, data, flag)
	u_int cmd;
	caddr_t data;
{
	register struct tty *tp;
	register unit = UNIT(dev);
	int error;

	tp = &dh11[unit];
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0) {
		if (cmd == TIOCSETP || cmd == TIOCSETN || cmd == TIOCLBIS ||
		    cmd == TIOCLBIC || cmd == TIOCLSET)
			dhparam(unit);
		return (error);
	}
	switch (cmd) {

	case TIOCSBRK:
		((struct dhdevice *)(tp->t_addr))->dhbreak |= 1<<(unit&017);
		break;

	case TIOCCBRK:
		((struct dhdevice *)(tp->t_addr))->dhbreak &= ~(1<<(unit&017));
		break;

	case TIOCSDTR:
		dmctl (unit, DML_DTR|DML_RTS, DMBIS);
		break;

	case TIOCCDTR:
		dmctl (unit, DML_DTR|DML_RTS, DMBIC);
		break;

	default:
		return (ENOTTY);
	}
	return (0);
}

/*
 * Set parameters from open or stty into the DH hardware
 * registers.
 */
dhparam(unit)
	register int unit;
{
	register struct tty *tp;
	register struct dhdevice *addr;
	register int lpar;
	int s;

	tp = &dh11[unit];
	addr = (struct dhdevice *)tp->t_addr;
	/*
	 * Block interrupts so parameters will be set
	 * before the line interrupts.
	 */
	s = spl5();
	addr->un.dhcsrl = (unit&0xf)|DH_IE;
	if ((tp->t_ispeed)==0) {
		tp->t_state |= TS_HUPCLS;
		dmctl(unit, DML_OFF, DMSET);
		splx(s);
		return;
	}
	lpar = ((tp->t_ospeed)<<10) | ((tp->t_ispeed)<<6);
	if ((tp->t_ispeed) == B134)
		lpar |= BITS6|PENABLE|HDUPLX;
	else if (tp->t_flags & (RAW|LITOUT|PASS8))
		lpar |= BITS8;
	else
		lpar |= BITS7|PENABLE;
	if ((tp->t_flags&EVENP) == 0)
		lpar |= OPAR;
	if ((tp->t_ospeed) == B110)
		lpar |= TWOSB;
	addr->dhlpr = lpar;
	splx(s);
}

/*
 * DH transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dhxint(dh)
	int dh;
{
	register struct tty *tp;
	register struct dhdevice *addr;
	short ttybit, bar, *sbar;
	struct uba_device *ui;
	register int unit;
	u_short cntr;

	ui = &dhinfo[dh];
	addr = (struct dhdevice *)ui->ui_addr;
	if (addr->un.dhcsr & DH_NXM) {
		addr->un.dhcsr |= DH_CNI;
		printf("dh%d:  NXM\n", dh);
	}
	sbar = &dhsar[dh];
	bar = *sbar & ~addr->dhbar;
	unit = dh * 16; ttybit = 1;
	addr->un.dhcsr &= (short)~DH_TI;
	for(; bar; unit++, ttybit <<= 1) {
		if(bar & ttybit) {
			*sbar &= ~ttybit;
			bar &= ~ttybit;
			tp = &dh11[unit];
			tp->t_state &= ~TS_BUSY;
			if (tp->t_state&TS_FLUSH)
				tp->t_state &= ~TS_FLUSH;
			else {
				addr->un.dhcsrl = (unit&017)|DH_IE;
#if !defined(UCB_CLIST) || defined(UNIBUS_MAP)
				/*
				 * Clists are either:
				 *	1)  in kernel virtual space,
				 *	    which in turn lies in the
				 *	    first 64K of physical memory or
				 *	2)  at UNIBUS virtual address 0.
				 *
				 * In either case, the extension bits are 0.
				 */
				cntr = (caddr_t)addr->dhcar - cpaddr(tp->t_outq.c_cf);
				ndflush(&tp->t_outq, (int)cntr);
#else
				{
				ubadr_t car;

				car = (ubadr_t) addr->dhcar
				    | (ubadr_t)(addr->dhsilo & 0300) << 10;
				cntr = car - cpaddr(tp->t_outq.c_cf);
				ndflush(&tp->t_outq, cntr);
				}
#endif
			}
			if (tp->t_line)
				(*linesw[tp->t_line].l_start)(tp);
			else
				dhstart(tp);
		}
	}
}

/*
 * Start (restart) transmission on the given DH11 line.
 */
dhstart(tp)
	register struct tty *tp;
{
	register struct dhdevice *addr;
	register int dh, unit, nch;
	int s;

	unit = UNIT(tp->t_dev);
	dh = unit >> 4;
	unit &= 0xf;
	addr = (struct dhdevice *)tp->t_addr;

	/*
	 * Must hold interrupts in following code to prevent
	 * state of the tp from changing.
	 */
	s = spl5();
	/*
	 * If it's currently active, or delaying, no need to do anything.
	 */
	if (tp->t_state&(TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
		goto out;
	/*
	 * If there are sleepers, and the output has drained below low
	 * water mark, wake up the sleepers.
	 */
	if (tp->t_outq.c_cc<=TTLOWAT(tp)) {
		if (tp->t_state&TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
				wakeup((caddr_t)&tp->t_outq);
		}
#ifdef UCB_NET
		if (tp->t_wsel) {
			selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
		}
#endif
	}
	/*
	 * Now restart transmission unless the output queue is
	 * empty.
	 */
	if (tp->t_outq.c_cc == 0)
		goto out;
	if (tp->t_flags & (RAW|LITOUT))
		nch = ndqb(&tp->t_outq, 0);
	else {
		nch = ndqb(&tp->t_outq, 0200);
		/*
		 * If first thing on queue is a delay, process it.
		 */
		if (nch == 0) {
			nch = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t) tp, (nch&0x7f)+6);
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	}
	/*
	 * If characters to transmit, restart transmission.
	 */
	if (nch) {
#if !defined(UCB_CLIST) || defined (UNIBUS_MAP)
		addr->un.dhcsrl = (char)((unit&017)|DH_IE);
		addr->dhcar = (u_short)cpaddr(tp->t_outq.c_cf);
#else
		ubadr_t uba;

		uba = cpaddr(tp->t_outq.c_cf);
		addr->un.dhcsrl = (unit&017) | DH_IE | ((hiint(uba)<<4)&060);
		addr->dhcar = loint(uba);
#endif
		{ short word = 1 << unit;
		dhsar[dh] |= word;
		addr->dhbcr = -nch;
		addr->dhbar |= word;
		}
		tp->t_state |= TS_BUSY;
	}
out:
	splx(s);
}


/*
 * Stop output on a line, e.g. for ^S/^Q or output flush.
 */
/*ARGSUSED*/
dhstop(tp, flag)
	register struct tty *tp;
{
	register struct dhdevice *addr;
	register int unit, s;

	addr = (struct dhdevice *)tp->t_addr;
	/*
	 * Block input/output interrupts while messing with state.
	 */
	s = spl5();
	if (tp->t_state & TS_BUSY) {
		/*
		 * Device is transmitting; stop output
		 * by selecting the line and setting the byte
		 * count to -1.  We will clean up later
		 * by examining the address where the dh stopped.
		 */
		unit = UNIT(tp->t_dev);
		addr->un.dhcsrl = (unit&017) | DH_IE;
		if ((tp->t_state & TS_TTSTOP) == 0) {
			tp->t_state |= TS_FLUSH;
		}
		addr->dhbcr = -1;
	}
	splx(s);
}

int dhtransitions, dhslowtimers, dhfasttimers;		/*DEBUG*/
/*
 * At software clock interrupt time, check status.
 * Empty all the dh silos that are in use, and decide whether
 * to turn any silos off or on.
 */
dhtimer()
{
	register int dh, s;
	static int timercalls;

	if (dhsilos) {
		dhfasttimers++;		/*DEBUG*/
		timercalls++;
		s = spl5();
		for (dh = 0; dh < NDH; dh++)
			if (dhsilos & (1 << dh))
				dhrint(dh);
		splx(s);
	}
	if ((dhsilos == 0) || ((timercalls += FASTTIMER) >= hz)) {
		dhslowtimers++;		/*DEBUG*/
		timercalls = 0;
		for (dh = 0; dh < NDH; dh++) {
		    ave(dhrate[dh], dhchars[dh], 8);
		    if ((dhchars[dh] > dhhighrate) &&
		      ((dhsilos & (1 << dh)) == 0)) {
			((struct dhdevice *)(dhinfo[dh].ui_addr))->dhsilo =
			    (dhchars[dh] > 500? 32 : 16);
			dhsilos |= (1 << dh);
			dhtransitions++;		/*DEBUG*/
		    } else if ((dhsilos & (1 << dh)) &&
		      (dhrate[dh] < dhlowrate)) {
			((struct dhdevice *)(dhinfo[dh].ui_addr))->dhsilo = 0;
			dhsilos &= ~(1 << dh);
		    }
		    dhchars[dh] = 0;
		}
	}
	timeout(dhtimer, (caddr_t) 0, dhsilos ? FASTTIMER : LINEHZ);
}

/*
 * Turn on the line associated with dh dev.
 */
dmopen(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct dmdevice *addr;
	register struct uba_device *ui;
	register int unit;
	register int dm;
	int s;

	unit = UNIT(dev);
	dm = unit >> 4;
	tp = &dh11[unit];
	unit &= 0xf;
	if (dev & 0200)
		dhsoftCAR[dm] |= (1<<(unit&0xf));
	else
		dhsoftCAR[dm] &= ~(1<<(unit&0xf));
	if (dm >= NDH || (ui = &dminfo[dm])->ui_alive == 0) {
		tp->t_state |= TS_CARR_ON;
		return;
	}
	addr = (struct dmdevice *)ui->ui_addr;
	s = spl5();
	addr->dmcsr &= ~DM_SE;
	while (addr->dmcsr & DM_BUSY)
		;
	addr->dmcsr = unit & 017;
	addr->dmlstat = DML_ON;
	if ((addr->dmlstat&DML_CAR) || (dhsoftCAR[dm]&(1<<unit)))
		tp->t_state |= TS_CARR_ON;
	addr->dmcsr = DM_IE|DM_SE;
	while ((tp->t_state & TS_CARR_ON)==0)
		sleep((caddr_t) &tp->t_rawq, TTIPRI);
	splx(s);
}

/*
 * Dump control bits into the DM registers.
 */
dmctl(unit, bits, how)
	int unit;
	int bits, how;
{
	register struct uba_device *ui;
	register struct dmdevice *addr;
	register s;
	int dm;

	dm = unit >> 4;
	if ((ui = &dminfo[dm])->ui_alive == 0)
		return;
	addr = (struct dmdevice *)ui->ui_addr;
	s = spl5();
	addr->dmcsr &= ~DM_SE;
	while (addr->dmcsr & DM_BUSY)
		;
	addr->dmcsr = unit & 0xf;
	switch (how) {
	case DMSET:
		addr->dmlstat = bits;
		break;
	case DMBIS:
		addr->dmlstat |= bits;
		break;
	case DMBIC:
		addr->dmlstat &= ~bits;
		break;
	}
	addr->dmcsr = DM_IE|DM_SE;
	splx(s);
}

/*
 * DM interrupt; deal with carrier transitions.
 */
dmintr(dm)
	register int dm;
{
	register struct uba_device *ui;
	register struct tty *tp;
	register struct dmdevice *addr;
	int unit;

	ui = &dminfo[dm];
	if (ui == 0)
		return;
	addr = (struct dmdevice *)ui->ui_addr;
	if (addr->dmcsr&DM_DONE) {
		if (addr->dmcsr&DM_CF) {
			unit = addr->dmcsr & 0xf;
			tp = &dh11[(dm << 4) + unit];
			if (addr->dmlstat & DML_CAR)
				(void)(*linesw[tp->t_line].l_modem)(tp, 1);
			else if ((dhsoftCAR[dm] & (1<<unit)) == 0 &&
			    (*linesw[tp->t_line].l_modem)(tp, 0) == 0)
				addr->dmlstat = 0;
		}
		addr->dmcsr = DM_IE|DM_SE;
	}
}
#endif
