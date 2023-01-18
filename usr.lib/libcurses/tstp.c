/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if !defined(lint) && !defined(NOSCCS)
static char sccsid[] = "@(#)tstp.c	5.1 (Berkeley) 6/7/85";
#endif

# include	<signal.h>

# include	"curses.ext"

/*
 * handle stop and start signals
 *
 * @(#)tstp.c	5.1 (Berkeley) 6/7/85
 */
tstp() {

# ifdef SIGTSTP

	SGTTY	tty;
	long	omask;
# ifdef DEBUG
	if (outf)
		fflush(outf);
# endif
	tty = _tty;
	mvcur(0, COLS - 1, LINES - 1, 0);
	endwin();
	fflush(stdout);
	/* reset signal handler so kill below stops us */
	signal(SIGTSTP, SIG_DFL);
	omask = sigsetmask(sigblock(0L) &~ sigmask(SIGTSTP));
	kill(0, SIGTSTP);
	sigblock(sigmask(SIGTSTP));
	signal(SIGTSTP, tstp);
	_tty = tty;
	stty(_tty_ch, &_tty);
	wrefresh(curscr);
# endif	SIGTSTP
}
