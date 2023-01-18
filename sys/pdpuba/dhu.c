/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)dhu.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * based on	dh.c 6.3	84/03/15
 * and on	dmf.c	6.2	84/02/16
 *
 * Dave Johnson, Brown University Computer Science
 *	ddj%brown@csnet-relay
 */

#include "dhu.h"
#if NDHU > 0
/*
 * DHU-11 driver
 */
#include "param.h"
#include "dhureg.h"
#include "conf.h"
#include "user.h"
#include "file.h"
#include "ioctl.h"
#include "tty.h"
#include "clist.h"
#include "map.h"
#include "uba.h"
#include "ubavar.h"
#include "systm.h"

struct	uba_device dhuinfo[NDHU];

#define	NDHULINE	(NDHU*16)

#define	UNIT(x)	(minor(x) & 0177)

#ifndef PORTSELECTOR
#define ISPEED	B9600
#define IFLAGS	(EVENP|ODDP|ECHO)
#else
#define ISPEED	B4800
#define IFLAGS	(EVENP|ODDP)
#endif

/*
 * default receive silo timeout value -- valid values are 2..255
 * number of ms. to delay between first char received and receive interrupt
 *
 * A value of 20 gives same response as ABLE dh/dm with silo alarm = 0
 */
#define	DHU_DEF_TIMO	20

/*
 * Other values for silo timeout register defined here but not used:
 * receive interrupt only on modem control or silo alarm (3/4 full)
 */
#define DHU_POLL_TIMO	0
/*
 * receive interrupt immediately on receive character
 */
#define DHU_NO_TIMO	1

/*
 * Local variables for the driver
 */
/*
 * Baud rates: no 50, 200, or 38400 baud; all other rates are from "Group B".
 *	EXTA => 19200 baud
 *	EXTB => 2000 baud
 */
char	dhu_speeds[] =
	{ 0, 0, 1, 2, 3, 4, 0, 5, 6, 7, 8, 10, 11, 13, 14, 9 };

short	dhusoftCAR[NDHU];

struct	tty dhu_tty[NDHULINE];
int	ndhu = NDHULINE;
int	dhuact;				/* mask of active dhu's */
int	dhustart(), ttrstrt();
long	dhumctl(),dmtodhu();

#if defined(UNIBUS_MAP) || defined(UCB_CLIST)
extern	ubadr_t clstaddr;
#define	cpaddr(x)	(clstaddr + (ubadr_t)((x) - (char *)cfree))
#else
#define	cpaddr(x)	((u_short)(x))
#endif

/*
 * Routine called to attach a dhu.
 * Called duattached for autoconfig.
 */
duattach(addr,unit)
	register caddr_t addr;
	register u_int unit;
{
	register struct uba_device *ui;

	if (addr && unit < NDHU && !dhuinfo[unit].ui_addr) {
		ui = &dhuinfo[unit];
		ui->ui_unit = unit;
		ui->ui_addr = addr;
		ui->ui_alive = 1;
		return (1);
	}
	return (0);
}

/*
 * Open a DHU11 line, mapping the clist onto the uba if this
 * is the first dhu on this uba.  Turn on this dhu if this is
 * the first use of it.
*/
/*ARGSUSED*/
dhuopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register int unit, dhu;
	register struct dhudevice *addr;
	register struct uba_device *ui;
	int s;

	unit = UNIT(dev);
	dhu = unit >> 4;
	if (dev & 0200)
		dhusoftCAR[dhu] |= (1<<(unit&0xf));
	else
		dhusoftCAR[dhu] &= ~(1<<(unit&0xf));
	if (unit >= NDHULINE || (ui = &dhuinfo[dhu])->ui_alive == 0)
		return (ENXIO);
	tp = &dhu_tty[unit];
	if (tp->t_state & TS_XCLUDE && u.u_uid != 0)
		return (EBUSY);
	addr = (struct dhudevice *)ui->ui_addr;
	tp->t_addr = (caddr_t)addr;
	tp->t_oproc = dhustart;
	/*
	 * While setting up state for this uba and this dhu,
	 * block uba resets which can clear the state.
	 */
	s = spl5();
	if ((dhuact&(1<<dhu)) == 0) {
		addr->dhucsr = DHU_SELECT(0) | DHU_IE;
		addr->dhutimo = DHU_DEF_TIMO;
		dhuact |= (1<<dhu);
		/* anything else to configure whole board */
	}
	(void) splx(s);
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
#endif PORTSELECTOR
		tp->t_dev = dev;
		dhuparam(unit);
	}
	/*
	 * Wait for carrier, then process line discipline specific open.
	 */
	s = spl5();
	if ((dhumctl(dev, (long)DHU_ON, DMSET) & DHU_CAR) ||
	    (dhusoftCAR[dhu] & (1<<(unit&0xf))))
		tp->t_state |= TS_CARR_ON;
	while ((tp->t_state & TS_CARR_ON) == 0) {
		tp->t_state |= TS_WOPEN;
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	(void) splx(s);
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*
 * Close a DHU11 line, turning off the modem control.
 */
/*ARGSUSED*/
dhuclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register unit;

	unit = UNIT(dev);
	tp = &dhu_tty[unit];
	(*linesw[tp->t_line].l_close)(tp);
	(void) dhumctl(unit, (long)DHU_BRK, DMBIC);
	if ((tp->t_state&(TS_HUPCLS|TS_WOPEN)) || (tp->t_state&TS_ISOPEN)==0)
#ifdef PORTSELECTOR
	{
		extern int wakeup();

		(void) dhumctl(unit, (long)DHU_OFF, DMSET);
		/* Hold DTR low for 0.5 seconds */
		timeout(wakeup, (caddr_t) &tp->t_dev, LINEHZ/2);
		sleep((caddr_t) &tp->t_dev, PZERO);
	}
#else
		(void) dhumctl(unit, DHU_OFF, DMSET);
#endif PORTSELECTOR
	ttyclose(tp);
}

dhuread(dev)
	dev_t dev;
{
	register struct tty *tp = &dhu_tty[UNIT(dev)];

	return ((*linesw[tp->t_line].l_read)(tp));
}

dhuwrite(dev)
	dev_t dev;
{
	register struct tty *tp = &dhu_tty[UNIT(dev)];

	return ((*linesw[tp->t_line].l_write)(tp));
}

/*
 * DHU11 receiver interrupt.
*/
dhurint(dhu)
	int dhu;
{
	register struct tty *tp;
	register c;
	register struct dhudevice *addr;
	register struct tty *tp0;
	register struct uba_device *ui;
	register line;
	int overrun = 0;

	ui = &dhuinfo[dhu];
	if (ui == 0 || ui->ui_alive == 0)
		return;
	addr = (struct dhudevice *)ui->ui_addr;
	tp0 = &dhu_tty[dhu<<4];
	/*
	 * Loop fetching characters from the silo for this
	 * dhu until there are no more in the silo.
	*/
	while ((c = addr->dhurbuf) < 0) {	/* (c & DHU_RB_VALID) == on */
		line = DHU_RX_LINE(c);
		tp = tp0 + line;
		if ((c & DHU_RB_STAT) == DHU_RB_STAT) {
			/*
			 * modem changed or diag info
			 */
			if (c & DHU_RB_DIAG) {
				/* decode diagnostic messages */
				continue;
			}
			if (c & DHU_ST_DCD)
				(void)(*linesw[tp->t_line].l_modem)(tp, 1);
			else if ((dhusoftCAR[dhu] & (1<<line)) == 0 &&
			    (*linesw[tp->t_line].l_modem)(tp, 0) == 0)
				(void) dhumctl((dhu<<4)|line, DHU_OFF, DMSET);
			continue;
		}
		if ((tp->t_state&TS_ISOPEN) == 0) {
			wakeup((caddr_t)&tp->t_rawq);
#ifdef PORTSELECTOR
			if ((tp->t_state&TS_WOPEN) == 0)
#endif
				continue;
		}
		if (c & DHU_RB_PE)
			if ((tp->t_flags&(EVENP|ODDP)) == EVENP ||
			    (tp->t_flags&(EVENP|ODDP)) == ODDP)
				continue;
		if ((c & DHU_RB_DO) && overrun == 0) {
			printf("dhu%d: silo overflow\n", dhu);
			overrun = 1;
		}
		if (c & DHU_RB_FE)
			/*
			 * At framing error (break) generate
			 * a null (in raw mode, for getty), or a
			 * interrupt (in cooked/cbreak mode).
			 */
			if (tp->t_flags&RAW)
				c = 0;
			else
				c = tp->t_intrc;
#if NBK > 0
		if (tp->t_line == NETLDISC) {
			c &= 0x7f;
			BKINPUT(c, tp);
		} else
#endif
			(*linesw[tp->t_line].l_rint)(c, tp);
	}
}

/*
 * Ioctl for DHU11.
 */
/*ARGSUSED*/
dhuioctl(dev, cmd, data, flag)
	u_int cmd;
	caddr_t data;
{
	register struct tty *tp;
	register int unit = UNIT(dev);
	int error;

	tp = &dhu_tty[unit];
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0) {
		if (cmd == TIOCSETP || cmd == TIOCSETN || cmd == TIOCLSET ||
		    cmd == TIOCLBIC || cmd == TIOCLBIS)
			dhuparam(unit);
		return (error);
	}

	switch (cmd) {
	case TIOCSBRK:
		(void) dhumctl(unit, (long)DHU_BRK, DMBIS);
		break;

	case TIOCCBRK:
		(void) dhumctl(unit, (long)DHU_BRK, DMBIC);
		break;

	case TIOCSDTR:
		(void) dhumctl(unit, (long)DHU_DTR|DHU_RTS, DMBIS);
		break;

	case TIOCCDTR:
		(void) dhumctl(unit, (long)DHU_DTR|DHU_RTS, DMBIC);
		break;

	case TIOCMSET:
		(void) dhumctl(dev, dmtodhu(*(int *)data), DMSET);
		break;

	case TIOCMBIS:
		(void) dhumctl(dev, dmtodhu(*(int *)data), DMBIS);
		break;

	case TIOCMBIC:
		(void) dhumctl(dev, dmtodhu(*(int *)data), DMBIC);
		break;

	case TIOCMGET:
		*(int *)data = dhutodm(dhumctl(dev, 0L, DMGET));
		break;
	default:
		return (ENOTTY);
	}
	return (0);
}

static long
dmtodhu(bits)
	register int bits;
{
	long b = 0;

	if (bits & DML_RTS) b |= DHU_RTS;
	if (bits & DML_DTR) b |= DHU_DTR;
	if (bits & DML_LE) b |= DHU_LE;
	return(b);
}

static
dhutodm(bits)
	long bits;
{
	register int b = 0;

	if (bits & DHU_DSR) b |= DML_DSR;
	if (bits & DHU_RNG) b |= DML_RNG;
	if (bits & DHU_CAR) b |= DML_CAR;
	if (bits & DHU_CTS) b |= DML_CTS;
	if (bits & DHU_RTS) b |= DML_RTS;
	if (bits & DHU_DTR) b |= DML_DTR;
	if (bits & DHU_LE) b |= DML_LE;
	return(b);
}

/*
 * Set parameters from open or stty into the DHU hardware
 * registers.
 */
static
dhuparam(unit)
	register int unit;
{
	register struct tty *tp;
	register struct dhudevice *addr;
	register int lpar;
	int s;

	tp = &dhu_tty[unit];
	addr = (struct dhudevice *)tp->t_addr;
	/*
	 * Block interrupts so parameters will be set
	 * before the line interrupts.
	 */
	s = spl5();
	if ((tp->t_ispeed) == 0) {
		tp->t_state |= TS_HUPCLS;
		(void)dhumctl(unit, (long)DHU_OFF, DMSET);
		splx(s);
		return;
	}
	lpar = (dhu_speeds[tp->t_ospeed]<<12) | (dhu_speeds[tp->t_ispeed]<<8);
	if ((tp->t_ispeed) == B134)
		lpar |= DHU_LP_BITS6|DHU_LP_PENABLE;
	else if (tp->t_flags & (RAW|LLITOUT|PASS8))
		lpar |= DHU_LP_BITS8;
	else
		lpar |= DHU_LP_BITS7|DHU_LP_PENABLE;
	if (tp->t_flags&EVENP)
		lpar |= DHU_LP_EPAR;
	if ((tp->t_ospeed) == B110)
		lpar |= DHU_LP_TWOSB;
	addr->dhucsr = DHU_SELECT(unit) | DHU_IE;
	addr->dhulpr = lpar;
	splx(s);
}

/*
 * DHU11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dhuxint(dhu)
	int dhu;
{
	register struct tty *tp;
	register struct dhudevice *addr;
	register struct tty *tp0;
	register struct uba_device *ui;
	register int line, t;
	u_short cntr;

	ui = &dhuinfo[dhu];
	tp0 = &dhu_tty[dhu<<4];
	addr = (struct dhudevice *)ui->ui_addr;
	while ((t = addr->dhucsrh) & DHU_CSH_TI) {
		line = DHU_TX_LINE(t);
		tp = tp0 + line;
		tp->t_state &= ~TS_BUSY;
		if (t & DHU_CSH_NXM) {
			printf("dhu(%d,%d): NXM fault\n", dhu, line);
			/* SHOULD RESTART OR SOMETHING... */
		}
		if (tp->t_state&TS_FLUSH)
			tp->t_state &= ~TS_FLUSH;
		else {
			addr->dhucsrl = DHU_SELECT(line) | DHU_IE;
			/*
			 * Clists are either:
			 *	1)  in kernel virtual space,
			 *	    which in turn lies in the
			 *	    first 64K of physical memory or
			 *	2)  at UNIBUS virtual address 0.
			 *
			 * In either case, the extension bits are 0.
			*/
#if !defined(UCB_CLIST) || defined(UNIBUS_MAP)
			cntr = addr->dhubar1 - cpaddr(tp->t_outq.c_cf);
#else
			/* UCB_CLIST && !UNIBUS_MAP (QBUS) taylor@oswego */
			{
			ubadr_t base;

			base = (ubadr_t) addr->dhubar1 | (ubadr_t)((addr->dhubar2 & 037) << 16);
			cntr = base - cpaddr(tp->t_outq.c_cf);
			}
#endif
			ndflush(&tp->t_outq,cntr);
		}
		if (tp->t_line)
			(*linesw[tp->t_line].l_start)(tp);
		else
			dhustart(tp);
	}
}

/*
 * Start (restart) transmission on the given DHU11 line.
 */
dhustart(tp)
	register struct tty *tp;
{
	register struct dhudevice *addr;
	register int unit, nch;
	ubadr_t car;
	int s;

	unit = minor(tp->t_dev);
	unit &= 0xf;
	addr = (struct dhudevice *)tp->t_addr;

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
	 * If there are sleepers, and output has drained below low
	 * water mark, wake up the sleepers..
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
		 * If first thing on queue is a delay process it.
		 */
		if (nch == 0) {
			nch = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (nch&0x7f)+6);
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	}
	/*
	 * If characters to transmit, restart transmission.
	 */
	if (nch) {
		car = cpaddr(tp->t_outq.c_cf);
		addr->dhucsrl = DHU_SELECT(unit) | DHU_IE;
		addr->dhulcr &= ~DHU_LC_TXABORT;
		addr->dhubcr = nch;
		addr->dhubar1 = loint(car);
#if !defined(UCB_CLIST) || defined(UNIBUS_MAP)
		addr->dhubar2 = (hiint(car) & DHU_BA2_XBA) | DHU_BA2_DMAGO;
#else
		/* UCB_CLIST && !UNIBUS_MAP (QBUS) taylor@oswego */
		addr->dhubar2 = (hiint(car) & 037) | DHU_BA2_DMAGO;
#endif
		tp->t_state |= TS_BUSY;
	}
out:
	splx(s);
}

/*
 * Stop output on a line, e.g. for ^S/^Q or output flush.
 */
/*ARGSUSED*/
dhustop(tp, flag)
	register struct tty *tp;
{
	register struct dhudevice *addr;
	register int unit, s;

	addr = (struct dhudevice *)tp->t_addr;
	/*
	 * Block input/output interrupts while messing with state.
	 */
	s = spl5();
	if (tp->t_state & TS_BUSY) {
		/*
		 * Device is transmitting; stop output
		 * by selecting the line and setting the
		 * abort xmit bit.  We will get an xmit interrupt,
		 * where we will figure out where to continue the
		 * next time the transmitter is enabled.  If
		 * TS_FLUSH is set, the outq will be flushed.
		 * In either case, dhustart will clear the TXABORT bit.
		 */
		unit = minor(tp->t_dev);
		addr->dhucsrl = DHU_SELECT(unit) | DHU_IE;
		addr->dhulcr |= DHU_LC_TXABORT;
		if ((tp->t_state&TS_TTSTOP)==0)
			tp->t_state |= TS_FLUSH;
	}
	(void) splx(s);
}

/*
 * DHU11 modem control
 */
static long
dhumctl(dev, bits, how)
	dev_t dev;
	long bits;
	int how;
{
	register struct dhudevice *dhuaddr;
	register int unit;
	long mbits;
	int s;

	unit = UNIT(dev);
	dhuaddr = (struct dhudevice *)(dhu_tty[unit].t_addr);
	unit &= 0xf;
	s = spl5();
	dhuaddr->dhucsr = DHU_SELECT(unit) | DHU_IE;
	/*
	 * combine byte from stat register (read only, bits 16..23)
	 * with lcr register (read write, bits 0..15).
	 */
	mbits = (u_short)dhuaddr->dhulcr | ((long)dhuaddr->dhustat << 16);
	switch (how) {

	case DMSET:
		mbits = (mbits & 0xff0000L) | bits;
		break;

	case DMBIS:
		mbits |= bits;
		break;

	case DMBIC:
		mbits &= ~bits;
		break;

	case DMGET:
		(void) splx(s);
		return(mbits);
	}
	dhuaddr->dhulcr = (mbits & 0xffff) | DHU_LC_RXEN;
	dhuaddr->dhulcr2 = DHU_LC2_TXEN;
	(void) splx(s);
	return(mbits);
}
#endif
