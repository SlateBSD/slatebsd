/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)subr_prf.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "user.h"
#include "buf.h"
#include "msgbuf.h"
#include "conf.h"
#include "ioctl.h"
#include "tty.h"
#include "reboot.h"
#include "systm.h"

#define TOCONS	0x1
#define TOTTY	0x2
#define TOLOG	0x4

/*
 * In case console is off,
 * panicstr contains argument to last
 * call to panic.
 */
char	*panicstr;

/*
 * Scaled down version of C Library printf.
 * Used to print diagnostic information directly on console tty.
 * Since it is not interrupt driven, all system activities are
 * suspended.  Printf should not be used for chit-chat.
 *
 * One additional format: %b is supported to decode error registers.
 * Usage is:
 *	printf("reg=%b\n", regval, "<base><arg>*");
 * Where <base> is the output base expressed as a control character,
 * e.g. \10 gives octal; \20 gives hex.  Each arg is a sequence of
 * characters, the first of which gives the bit number to be inspected
 * (origin 1), and the next characters (up to a control character, i.e.
 * a character <= 32), give the name of the register.  Thus
 *	printf("reg=%b\n", 3, "\10\2BITTWO\1BITONE\n");
 * would produce output:
 *	reg=3<BITTWO,BITONE>
 */

/*VARARGS1*/
printf(fmt, x1)
	char *fmt;
	unsigned x1;
{
	prf(fmt, &x1, TOCONS | TOLOG);
}

/*
 * Uprintf prints to the current user's terminal,
 * guarantees not to sleep (so could be called by interrupt routines;
 * but prints on the tty of the current process)
 * and does no watermark checking - (so no verbose messages).
 * NOTE: with current kernel mapping scheme, the user structure is
 * not guaranteed to be accessible at interrupt level (see seg.h);
 * a savemap/restormap would be needed here or in putchar if uprintf
 * was to be used at interrupt time.
 */
/*VARARGS1*/
uprintf(fmt, x1)
	char	*fmt;
	unsigned x1;
{
	register struct tty *tp;

	if ((tp = u.u_ttyp) == NULL)
		return;

	if (ttycheckoutq(tp, 1))
		prf(fmt, &x1, TOTTY);
}

prf(fmt, adx, flags)
	register char *fmt;
	register u_int *adx;
	int flags;
{
	register int c;
	u_int b;
	char *s;
	int	i, any;

loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0')
			return;
		putchar(c, flags);
	}
	c = *fmt++;
	switch (c) {

	case 'l':
		c = *fmt++;
		switch(c) {
			case 'x':
				b = 16;
				goto lnumber;
			case 'd':
				b = 10;
				goto lnumber;
			case 'o':
				b = 8;
				goto lnumber;
			default:
				putchar('%', flags);
				putchar('l', flags);
				putchar(c, flags);
		}
		break;
	case 'X':
		b = 16;
		goto lnumber;
	case 'D':
		b = 10;
		goto lnumber;
	case 'O':
		b = 8;
lnumber:	printn(*(long *)adx, b, flags);
		adx += (sizeof(long) / sizeof(int)) - 1;
		break;
	case 'x':
		b = 16;	
		goto number;
	case 'd':
	case 'u':		/* what a joke */
		b = 10;
		goto number;
	case 'o':
		b = 8;
number:		printn((long)*adx, b, flags);
		break;
	case 'c':
		putchar(*adx, flags);
		break;
	case 'b':
		b = *adx++;
		s = (char *)*adx;
		printn((long)b, *s++, flags);
		any = 0;
		if (b) {
			while (i = *s++) {
				if (b & (1 << (i - 1))) {
					putchar(any? ',' : '<', flags);
					any = 1;
					for (; (c = *s) > 32; s++)
						putchar(c, flags);
				} else
					for (; *s > 32; s++)
						;
			}
			if (any)
				putchar('>', flags);
		}
		break;
	case 's':
		s = (char *)*adx;
		while (c = *s++)
			putchar(c, flags);
		break;
	case '%':
		putchar(c, flags);
		break;
	default:
		putchar('%', flags);
		putchar(c, flags);
		break;
	}
	adx++;
	goto loop;
}

/*
 * Printn prints a number n in base b.
 * We don't use recursion to avoid deep kernels stacks.
 */
printn(n, b, flags)
	long n;
	u_int b;
{
	char prbuf[12];
	register char *cp = prbuf;
	register int offset = 0;

	if (n<0)
		switch(b) {
		case 8:		/* unchecked, but should be like hex case */
		case 16:
			offset = b-1;
			n++;
			break;
		case 10:
			putchar('-', flags);
			n = -n;
			break;
		}
	do {
		*cp++ = "0123456789ABCDEF"[offset + n%b];
	} while (n = n/b);	/* Avoid  n /= b, since that requires alrem */
	do
		putchar(*--cp, flags);
	while (cp > prbuf);
}

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then reboots.
 * If we are called twice, then we avoid trying to
 * sync the disks as this often leads to recursive panics.
 */
panic(s)
	char *s;
{
	int bootopt = RB_AUTOBOOT|RB_DUMP;

	if (panicstr)
		bootopt |= RB_NOSYNC;
	else {
		panicstr = s;
	}
	printf("panic: %s\n", s);
	boot(rootdev, bootopt);
}

/*
 * Warn that a system table is full.
 */
tablefull(tab)
	char *tab;
{
	printf("%s: table is full\n", tab);
}

/*
 * Hard error is the preface to plaintive error messages
 * about failing disk tranfers.
 */
harderr(bp, cp)
	struct buf *bp;
	char *cp;
{
	printf("%s%d%c: hard error sn%D ", cp,
	    minor(bp->b_dev) >> 3, 'a'+(minor(bp->b_dev) & 07), bp->b_blkno);
}

/*
 * Print a character on console or users terminal.
 * If destination is console then the last MSGBUFS characters
 * are saved in msgbuf for inspection later.
 */
putchar(c, flags)
	register int c;
{
	extern char *panicstr;

	if (flags & TOTTY) {
		register int s = spltty();
		register struct tty *tp = u.u_ttyp;

		if (tp && (tp->t_state & (TS_CARR_ON | TS_ISOPEN)) ==
			(TS_CARR_ON | TS_ISOPEN)) {
			if (c == '\n')
				(void) ttyoutput('\r', tp);
			(void) ttyoutput(c, tp);
			ttstart(tp);
		}
		splx(s);
	}
	if ((flags & TOLOG) && c != '\0' && c != '\r' && c != 0177) {
		msgbuf.msg_bufc[msgbuf.msg_bufx++] = c;
		if (msgbuf.msg_bufx < 0 || msgbuf.msg_bufx >= MSG_BSIZE)
			msgbuf.msg_bufx = 0;
	}
	if ((flags & TOCONS) && c != '\0')
		cnputc(c);
}
