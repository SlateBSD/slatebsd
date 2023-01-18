/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_synch.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "user.h"
#include "proc.h"
#include "buf.h"
#include "signal.h"
#include "vm.h"
#include "kernel.h"
#include "systm.h"

#ifdef SMALL
#define	SQSIZE	010	/* Must be power of 2 */
#else
#define	SQSIZE	0100	/* Must be power of 2 */
#endif

#define	HASH(x)	(((int)x >> 5) & (SQSIZE - 1))
#define	SCHMAG	8/10

struct	proc *slpque[SQSIZE];

/*
 * Recompute process priorities, once a second
 */
schedcpu()
{
	register struct proc *p;
	register int a;

	wakeup((caddr_t)&lbolt);
	for (p = allproc; p != NULL; p = p->p_nxt) {
		if (p->p_time != 127)
			p->p_time++;
		/*
		 * this is where 2.10 does its real time alarms.  4.X uses
		 * timeouts, since it offers better than second resolution.
		 * Putting it here allows us to continue using use an int
		 * to store the number of ticks in the callout structure,
		 * since the kernel never has a timeout of greater than
		 * around 9 minutes.
		 */
		if (p->p_realtimer.it_value && !--p->p_realtimer.it_value) {
			psignal(p, SIGALRM);
			p->p_realtimer.it_value = p->p_realtimer.it_interval;
		}
		if (p->p_stat == SSLEEP || p->p_stat == SSTOP)
			if (p->p_slptime != 127)
				p->p_slptime++;
		if (p->p_slptime > 1)
			continue;
		a = (p->p_cpu & 0377) * SCHMAG + p->p_nice;
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		p->p_cpu = a;
		if (p->p_pri >= PUSER)
			setpri(p);
	}
	vmmeter();
	if (runin!=0) {
		runin = 0;
		wakeup((caddr_t)&runin);
	}
	++runrun;			/* swtch at least once a second */
	timeout(schedcpu, (caddr_t)0, LINEHZ);
}

/*
 * Recalculate the priority of a process after it has slept for a while.
 */
updatepri(p)
	register struct proc *p;
{
	register int a = p->p_cpu & 0377;

	p->p_slptime--;		/* the first time was done in schedcpu */
	while (a && --p->p_slptime)
		a = (SCHMAG * a) /* + p->p_nice */;
	if (a < 0)
		a = 0;
	if (a > 255)
		a = 255;
	p->p_cpu = a;
	(void) setpri(p);
}

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=PZERO a signal cannot disturb the sleep;
 * if pri>PZERO signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
sleep(chan, pri)
	caddr_t chan;
	int pri;
{
	register struct proc *rp;
	register struct proc **qp;
	register s;

	rp = u.u_procp;
	s = splhigh();
	if (panicstr) {
		/*
		 * After a panic, just give interrupts a chance, then just
		 * return; don't run any other procs or panic below, in
		 * case this is the idle process and already asleep.  The
		 * splnet should be spl0 if the network was being used
		 * by the filesystem, but for now avoid network interrupts
		 * that might cause another panic.
		 */
		(void)splnet();
		noop();
		splx(s);
		return;
	}
	if (!chan || rp->p_stat != SRUN)
		panic("sleep");
	rp->p_wchan = chan;
	rp->p_slptime = 0;
	rp->p_pri = pri;
	qp = &slpque[HASH(chan)];
	rp->p_link = *qp;
	*qp = rp;
	if (pri > PZERO) {
		/*
		 * If we stop in issig(), wakeup may already have happened
		 * when we return (rp->p_wchan will then be 0).
		 */
		if (ISSIG(rp)) {
			if (rp->p_wchan)
				unsleep(rp);
			rp->p_stat = SRUN;
			(void) spl0();
			goto psig;
		}
		if (rp->p_wchan == 0)
			goto out;
		rp->p_stat = SSLEEP;
		(void) spl0();
		/*
		 * maybe a very small core memory, give swapped out
		 * processes a chance.
		 */
		if (runin != 0) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
#ifdef UCB_RUSAGE
		u.u_ru.ru_nvcsw++;
#endif
		swtch();
		if (ISSIG(rp))
			goto psig;
	} else {
		rp->p_stat = SSLEEP;
		(void) spl0();
#ifdef UCB_RUSAGE
		u.u_ru.ru_nvcsw++;
#endif
		swtch();
	}
out:
	splx(s);
	return;

	/*
	 * If priority was low (>PZERO) and there has been a signal,
	 * execute non-local goto through u.u_qsave, aborting the
	 * system call in progress (see trap.c)
	 */
psig:
	longjmp(u.u_procp->p_addr, &u.u_qsave);
	/*NOTREACHED*/
}

/*
 * Remove a process from its wait queue
 */
unsleep(p)
	register struct proc *p;
{
	register struct proc **hp;
	register int s;

	s = splhigh();
	if (p->p_wchan) {
		hp = &slpque[HASH(p->p_wchan)];
		while (*hp != p)
			hp = &(*hp)->p_link;
		*hp = p->p_link;
		p->p_wchan = 0;
	}
	splx(s);
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
	register caddr_t chan;
{
	register struct proc *p, **q;
	struct proc **qp;
	int s;
#ifndef NOKA5
	mapinfo map;

	/*
	 * Since we are called at interrupt time, must insure normal
	 * kernel mapping to access proc.
	 */
	savemap(map);
#endif
	s = splclock();
	qp = &slpque[HASH(chan)];
restart:
	for (q = qp; p = *q; ) {
		if (p->p_stat != SSLEEP && p->p_stat != SSTOP)
			panic("wakeup");
		if (p->p_wchan==chan) {
			p->p_wchan = 0;
			*q = p->p_link;
			if (p->p_stat == SSLEEP) {
				/* OPTIMIZED INLINE EXPANSION OF setrun(p) */
				if (p->p_slptime > 1)
					updatepri(p);
				p->p_slptime = 0;
				p->p_stat = SRUN;
				if (p->p_flag & SLOAD)
					setrq(p);
#ifdef CGL_RTP
				if (p == rtpp)
					wantrtp++;
#endif
				/*
				 * Since curpri is a usrpri,
				 * p->p_pri is always better than curpri.
				 */
				runrun++;
				if ((p->p_flag&SLOAD) == 0) {
					if (runout != 0) {
						runout = 0;
						wakeup((caddr_t)&runout);
					}
				}
				/* END INLINE EXPANSION */
				goto restart;
			}
			p->p_slptime = 0;
		} else
			q = &p->p_link;
	}
	splx(s);
#ifndef NOKA5
	restormap(map);
#endif
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
	register struct proc *p;
{
	register int s;

	s = splhigh();
	switch (p->p_stat) {

	case 0:
	case SWAIT:
	case SRUN:
	case SZOMB:
	default:
		panic("setrun");

	case SSTOP:
	case SSLEEP:
		unsleep(p);		/* e.g. when sending signals */
		break;

	case SIDL:
		break;
	}
	if (p->p_slptime > 1)
		updatepri(p);
	p->p_stat = SRUN;
	if (p->p_flag & SLOAD)
		setrq(p);
	splx(s);
#ifdef CGL_RTP
	if (p == rtpp)
		wantrtp++;
	if (p->p_pri < curpri || p == rtpp)
		runrun++;
#else
	if (p->p_pri < curpri)
		runrun++;
#endif
	if ((p->p_flag&SLOAD) == 0) {
		if (runout != 0) {
			runout = 0;
			wakeup((caddr_t)&runout);
		}
	}
}

/*
 * Set user priority.
 * The rescheduling flag (runrun)
 * is set if the priority is better
 * than the currently running process.
 */
setpri(pp)
	register struct proc *pp;
{
	register int p;

	p = (pp->p_cpu & 0377)/16;
	p += PUSER + pp->p_nice;
	if (p > 127)
		p = 127;
	if (p < curpri)
		runrun++;
	pp->p_pri = p;
	return (p);
}

/*
 * This routine is called to reschedule the CPU.  If the calling process is
 * not in RUN state, arrangements for it to restart must have been made
 * elsewhere, usually by calling via sleep.  There is a race here.  A process
 * may become ready after it has been examined.  In this case, idle() will be
 * called and will return in at most 1hz time, e.g. it's not worth putting an
 * spl() in.
 */
swtch()
{
	register struct proc *p, *q;
	register int n;
	struct proc *pp, *pq;
	int s;

#ifdef DIAGNOSTIC
	extern struct buf *hasmap;
	if (hasmap)
		panic("swtch hasmap");
#endif
#ifdef UCB_METER
	cnt.v_swtch++;
#endif
	/* If not the idle process, resume the idle process. */
	if (u.u_procp != &proc[0]) {
		if (setjmp(&u.u_rsave)) {
			sureg();
			return;
		}
#ifndef NONFP
		if (!u.u_fpsaved) {
			savfp(&u.u_fps);
			u.u_fpsaved = 1;
		}
#endif
		longjmp(proc[0].p_addr, &u.u_qsave);
	}
	/*
	 * The first save returns nonzero when proc 0 is resumed
	 * by another process (above); then the second is not done
	 * and the process-search loop is entered.
	 *
	 * The first save returns 0 when swtch is called in proc 0
	 * from sched().  The second save returns 0 immediately, so
	 * in this case too the process-search loop is entered.
	 * Thus when proc 0 is awakened by being made runnable, it will
	 * find itself and resume itself at rsave, and return to sched().
	 */
	if (setjmp(&u.u_qsave)==0 && setjmp(&u.u_rsave))
		return;
loop:
	s = splhigh();
	noproc = 0;
	runrun = 0;
#ifdef CGL_RTP
	/*
	 * Test for the presence of a "real time process".
	 * If there is one and it is runnable, give it top priority.
	 */
	if ((p = rtpp) && p->p_stat == SRUN && (p->p_flag & SLOAD)) {
		pq = NULL;
		for (q = qs;;q = q->p_link) {
			if (q == NULL)
				panic("rtp not found\n");
			if (q == p)
				break;
			pq = q;
		}
		n = PRTP;
		wantrtp = 0;
		goto runem;
	}
#endif CGL_RTP
#ifdef DIAGNOSTIC
	for (p = qs; p; p = p->p_link)
		if (p->p_stat != SRUN)
			panic("swtch SRUN");
#endif
	pp = NULL;
	q = NULL;
	n = 128;
	/*
	 * search for highest-priority runnable process
	 */
	for (p = qs; p; p = p->p_link) {
		if (p->p_flag & SLOAD && p->p_pri < n) {
			pp = p;
			pq = q;
			n = p->p_pri;
		}
		q = p;
	}
	/*
	 * if no process is runnable, idle.
	 */
	p = pp;
	if (p == NULL) {
#ifdef UCB_FRCSWAP
		idleflg = 1;
#endif
		idle();
		goto loop;
	}
#ifdef CGL_RTP
runem:
#endif
	if (pq)
		pq->p_link = p->p_link;
	else
		qs = p->p_link;
	curpri = n;
	splx(s);
	/*
	 * the rsave (ssave) contents are interpreted
	 * in the new address space
	 */
	n = p->p_flag & SSWAP;
	p->p_flag &= ~SSWAP;
	longjmp(p->p_addr, n ? &u.u_ssave: &u.u_rsave);
}

setrq(p)
	register struct proc *p;
{
	register int s;

	s = splhigh();
#ifdef DIAGNOSTIC
	{			/* see if already on the run queue */
		register struct proc *q;

		for (q = qs;q != NULL;q = q->p_link)
			if (q == p)
				panic("setrq");
	}
#endif
	p->p_link = qs;
	qs = p;
	splx(s);
}

/*
 * Remove runnable job from run queue.  This is done when a runnable job
 * is swapped out so that it won't be selected in swtch().  It will be
 * reinserted in the qs with setrq when it is swapped back in.
 */
remrq(p)
	register struct proc *p;
{
	register struct proc *q;
	register int s;

	s = splhigh();
	if (p == qs)
		qs = p->p_link;
	else {
		for (q = qs; q; q = q->p_link)
			if (q->p_link == p) {
				q->p_link = p->p_link;
				goto done;
			}
			panic("remrq");
	}
done:
	splx(s);
}
