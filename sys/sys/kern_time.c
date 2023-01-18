/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_time.c	1.2 (2.10BSD Berkeley) 3/30/87
 */

#include "param.h"
#include "user.h"
#include "proc.h"
#include "kernel.h"

/* 
 * Time of day and interval timer support.
 *
 * These routines provide the kernel entry points to get and set
 * the time-of-day.
 */

gettimeofday()
{
	register struct a {
		struct	timeval *tp;
		struct	timezone *tzp;
	} *uap = (struct a *)u.u_ap;
	struct timeval atv;
	int s;
	register u_int	ms;

	if (uap->tp) {
		/*
		 * We don't resolve the milliseconds on every clock tick; it's
		 * easier to do it here, have to check to see if lbolt has
		 * rolled over anyway.  Long casts are out of paranoia.
		 */
		s = _spl7(); atv = time; ms = lbolt; splx(s);
		if (ms > LINEHZ) {
			ms -= LINEHZ;
			++atv.tv_sec;
		}
		atv.tv_usec = (long)ms * 1000000L / (long)LINEHZ;
		u.u_error = copyout((caddr_t)&atv, (caddr_t)(uap->tp),
			sizeof(atv));
		if (u.u_error)
			return;
	}
	if (uap->tzp)
		u.u_error = copyout((caddr_t)&tz, (caddr_t)uap->tzp,
 			sizeof (tz));
}

settimeofday()
{
	register struct a {
		struct	timeval *tv;
		struct	timezone *tzp;
	} *uap = (struct a *)u.u_ap;
	struct timeval atv;
	struct timezone atz;

	if (uap->tv) {
		u.u_error = copyin((caddr_t)uap->tv, (caddr_t)&atv,
			sizeof (struct timeval));
		if (u.u_error)
			return;
		setthetime(&atv);
	}
	if (uap->tzp && suser()) {
		u.u_error = copyin((caddr_t)uap->tzp, (caddr_t)&atz,
			sizeof (atz));
		if (u.u_error == 0)
			tz = atz;
	}
}

setthetime(tv)
	struct timeval *tv;
{
	int s;

	if (!suser())
		return;
/* WHAT DO WE DO ABOUT PENDING REAL-TIME TIMEOUTS??? */
	boottime.tv_sec += tv->tv_sec - time.tv_sec;
	s = spl7(); time = *tv; splx(s);
#ifndef BSD2_10
	/*
	 * if you have a time of day board, use it here
	 */
	resettodr();
#endif
}

/*
 * 2.10 adjtime simply increments/decrements immediately, resulting
 * in olddelta always being zero.  This is because we don't want to
 * make hardclock() do any more processing than necessary.
 */
adjtime()
{
	register struct a {
		struct timeval *delta;
		struct timeval *olddelta;
	} *uap = (struct a *)u.u_ap;
	struct timeval atv;
	register int s;

	if (!suser()) 
		return;
	u.u_error = copyin((caddr_t)uap->delta, (caddr_t)&atv,
		sizeof (struct timeval));
	if (u.u_error)
		return;
	s = splclock();
	time.tv_sec += atv.tv_sec;
	/*
	 * The kernel doesn't do divides by large longs.
	 *	lbolt += time.tv_usec * LINEHZ / 1000000L;
	 */
	splx(s);

	if (uap->olddelta) {
		atv.tv_sec = atv.tv_usec = 0;
		(void) copyout((caddr_t)&atv, (caddr_t)uap->olddelta,
			sizeof (struct timeval));
	}
}

getitimer()
{
	register struct a {
		u_int	which;
		struct	itimerval *itv;
	} *uap = (struct a *)u.u_ap;
	struct itimerval aitv;
	register int s;

	if (uap->which > ITIMER_PROF) {
		u.u_error = EINVAL;
		return;
	}
	aitv.it_interval.tv_usec = 0;
	aitv.it_value.tv_usec = 0;
	s = splclock();
	if (uap->which == ITIMER_REAL) {
		register struct proc *p = u.u_procp;

		aitv.it_interval.tv_sec = p->p_realtimer.it_interval;
		aitv.it_value.tv_sec = p->p_realtimer.it_value;
	}
	else {
		register struct k_itimerval *t = &u.u_timer[uap->which - 1];

		aitv.it_interval.tv_sec = t->it_interval / LINEHZ;
		aitv.it_value.tv_sec = t->it_value / LINEHZ;
	}
	splx(s);
	u.u_error = copyout((caddr_t)&aitv, (caddr_t)uap->itv,
	    sizeof (struct itimerval));
}

setitimer()
{
	register struct a {
		u_int	which;
		struct	itimerval *itv, *oitv;
	} *uap = (struct a *)u.u_ap;
	struct itimerval aitv;
	register struct itimerval *aitvp;
	int s;

	if (uap->which > ITIMER_PROF) {
		u.u_error = EINVAL;
		return;
	}
	aitvp = uap->itv;
	if (uap->oitv) {
		uap->itv = uap->oitv;
		getitimer();
	}
	if (aitvp == 0)
		return;
	u.u_error = copyin((caddr_t)aitvp, (caddr_t)&aitv,
	    sizeof (struct itimerval));
	if (u.u_error)
		return;
	s = splclock();
	if (uap->which == ITIMER_REAL) {
		register struct proc *p = u.u_procp;

		p->p_realtimer.it_value = aitv.it_value.tv_sec;
		if (aitv.it_value.tv_usec)
			++p->p_realtimer.it_value;
		p->p_realtimer.it_interval = aitv.it_interval.tv_sec;
		if (aitv.it_interval.tv_usec)
			++p->p_realtimer.it_interval;
	}
	else {
		register struct k_itimerval *t = &u.u_timer[uap->which - 1];

		t->it_value = aitv.it_value.tv_sec * LINEHZ;
		if (aitv.it_value.tv_usec)
			t->it_value += LINEHZ;
		t->it_interval = aitv.it_interval.tv_sec * LINEHZ;
		if (aitv.it_interval.tv_usec)
			t->it_interval += LINEHZ;
	}
	splx(s);
}

/*
 * Check that a proposed value to load into the .it_value or
 * .it_interval part of an interval timer is acceptable, and
 * fix it to have at least minimal value (i.e. if it is less
 * than the resolution of the clock, round it up.)
 */
itimerfix(tv)
	struct timeval *tv;
{

	if (tv->tv_sec < 0 || tv->tv_sec > 100000000L ||
	    tv->tv_usec < 0 || tv->tv_usec >= 1000000L)
		return (EINVAL);
	if (tv->tv_sec == 0 && tv->tv_usec != 0 && tv->tv_usec < (1000/hz))
		tv->tv_usec = 1000/hz;
	return (0);
}

#ifdef NOT_CURRENTLY_IN_USE
/*
 * Decrement an interval timer by a specified number
 * of microseconds, which must be less than a second,
 * i.e. < 1000000.  If the timer expires, then reload
 * it.  In this case, carry over (usec - old value) to
 * reducint the value reloaded into the timer so that
 * the timer does not drift.  This routine assumes
 * that it is called in a context where the timers
 * on which it is operating cannot change in value.
 */
itimerdecr(itp, usec)
	register struct itimerval *itp;
	int usec;
{

	if (itp->it_value.tv_usec < usec) {
		if (itp->it_value.tv_sec == 0) {
			/* expired, and already in next interval */
			usec -= itp->it_value.tv_usec;
			goto expire;
		}
		itp->it_value.tv_usec += 1000000;
		itp->it_value.tv_sec--;
	}
	itp->it_value.tv_usec -= usec;
	usec = 0;
	if (timerisset(&itp->it_value))
		return (1);
	/* expired, exactly at end of interval */
expire:
	if (timerisset(&itp->it_interval)) {
		itp->it_value = itp->it_interval;
		itp->it_value.tv_usec -= usec;
		if (itp->it_value.tv_usec < 0) {
			itp->it_value.tv_usec += 1000000L;
			itp->it_value.tv_sec--;
		}
	} else
		itp->it_value.tv_usec = 0;		/* sec is already 0 */
	return (0);
}
#endif /* NOT_CURRENTLY_IN_USE */

#define	timevalfix	tvfix

/*
 * Add and subtract routines for timevals.
 * N.B.: subtract routine doesn't deal with
 * results which are before the beginning,
 * it just gets very confused in this case.
 * Caveat emptor.
 */
timevaladd(t1, t2)
	struct timeval *t1, *t2;
{

	t1->tv_sec += t2->tv_sec;
	t1->tv_usec += t2->tv_usec;
	timevalfix(t1);
}

#ifdef NOT_CURRENTLY_IN_USE
timevalsub(t1, t2)
	struct timeval *t1, *t2;
{

	t1->tv_sec -= t2->tv_sec;
	t1->tv_usec -= t2->tv_usec;
	timevalfix(t1);
}
#endif

timevalfix(t1)
	struct timeval *t1;
{

	if (t1->tv_usec < 0) {
		t1->tv_sec--;
		t1->tv_usec += 1000000;
	}
	if (t1->tv_usec >= 1000000) {
		t1->tv_sec++;
		t1->tv_usec -= 1000000;
	}
}
