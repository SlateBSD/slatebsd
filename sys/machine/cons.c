/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)cons.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * KL/DL-11 driver
 */
#include "param.h"
#include "conf.h"
#include "user.h"
#include "proc.h"
#include "ioctl.h"
#include "tty.h"
#include "systm.h"
#include "cons.h"
#include "cn.h"

/*
 * Normal addressing:
 * minor 0 addresses KLADDR
 * minor 1 thru n-1 address from KLBASE (0176600),
 *    where n is the number of additional KL11's
 * minor n on address from DLBASE (0176500)
 */

struct dldevice *cnaddr = (struct dldevice *)0177560;

int	nkl11 = NKL;			/* for pstat */
struct	tty cons[NKL];
int	cnstart();
int	ttrstrt();
char	partab[];

#ifdef notdef
cnattach(addr, unit)
	struct dldevice *addr;
{
	if ((u_int)unit <= NKL) {
		cons[unit].t_addr = (caddr_t)addr;
		return (1);
	}
	return (0);
}
#endif

/*ARGSUSED*/
cnopen(dev, flag)
	dev_t dev;
{
	register struct dldevice *addr;
	register struct tty *tp;
	register int d;

	d = minor(dev);
	tp = &cons[d];
	if (!d && !tp->t_addr)
		tp->t_addr = (caddr_t)cnaddr;
	if (d >= NKL || !(addr = (struct dldevice *)tp->t_addr))
		return (ENXIO);
	tp->t_oproc = cnstart;
	if ((tp->t_state&TS_ISOPEN) == 0) {
		ttychars(tp);
		tp->t_state = TS_ISOPEN|TS_CARR_ON;
		tp->t_flags = EVENP|ECHO|XTABS|CRMOD;
	}
	if (tp->t_state&TS_XCLUDE && u.u_uid != 0)
		return (EBUSY);
	addr->dlrcsr |= DL_RIE|DL_DTR|DL_RE;
	addr->dlxcsr |= DLXCSR_TIE;
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*ARGSUSED*/
cnclose(dev, flag)
	dev_t dev;
{
	register struct tty *tp = &cons[minor(dev)];

	(*linesw[tp->t_line].l_close)(tp);
	ttyclose(tp);
}

/*ARGSUSED*/
cnread(dev)
	dev_t dev;
{
	register struct tty *tp = &cons[minor(dev)];

	return ((*linesw[tp->t_line].l_read)(tp));
}

/*ARGSUSED*/
cnwrite(dev)
	dev_t dev;
{
	register struct tty *tp = &cons[minor(dev)];

	return ((*linesw[tp->t_line].l_write)(tp));
}

/*ARGSUSED*/
cnrint(dev)
	dev_t dev;
{
	register int c;
	register struct dldevice *addr;
	register struct tty *tp = &cons[minor(dev)];

	addr = (struct dldevice *)tp->t_addr;
	c = addr->dlrbuf;
	addr->dlrcsr |= DL_RE;
	(*linesw[tp->t_line].l_rint)(c, tp);
}

/*ARGSUSED*/
cnioctl(dev, cmd, addr, flag)
	dev_t dev;
	register u_int cmd;
	caddr_t addr;
{
	register struct tty *tp = &cons[minor(dev)];
	register int error;

	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, addr, flag);
	if (error < 0)
		error = ENOTTY;
	return (error);
}

cnxint(dev)
	dev_t dev;
{
	register struct tty *tp = &cons[minor(dev)];

	tp->t_state &= ~TS_BUSY;
	(*linesw[tp->t_line].l_start)(tp);
}

cnstart(tp)
	register struct tty *tp;
{
	register struct dldevice *addr;
	register int c, s;

	s = spl5();
	if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
		goto out;
	if (tp->t_outq.c_cc <= TTLOWAT(tp)) {
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
	if (tp->t_outq.c_cc == 0)
		goto out;
	addr = (struct dldevice *)tp->t_addr;
	if ((addr->dlxcsr & DLXCSR_TRDY) == 0)
		return;
	c = getc(&tp->t_outq);
	if (tp->t_flags & (RAW|LITOUT))
		addr->dlxbuf = c&0xff;
	else if (c <= 0177)
		addr->dlxbuf = (c | ((partab[c]&0200))&0xff);
	else {
		timeout(ttrstrt, (caddr_t)tp, c&0177);
		tp->t_state |= TS_TIMEOUT;
		goto out;
	}
	tp->t_state |= TS_BUSY;
out:
	splx(s);
}

cnputc(c)
	register int c;
{
	register int s, timo;

	timo = 30000;
	/*
	 * Try waiting for the console tty to come ready,
	 * otherwise give up after a reasonable time.
	 */
	while ((cnaddr->dlxcsr & DLXCSR_TRDY) == 0)
		if (--timo == 0)
			break;
	if (c == 0)
		return;
	s = cnaddr->dlxcsr;
	cnaddr->dlxcsr = 0;
	cnaddr->dlxbuf = c&0xff;
	if (c == '\n')
		cnputc('\r');
	cnputc(0);
	cnaddr->dlxcsr = s;
}
