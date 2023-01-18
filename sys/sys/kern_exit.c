/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_exit.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/psl.h"
#include "../machine/reg.h"

#include "systm.h"
#include "map.h"
#include "user.h"
#include "proc.h"
#include "inode.h"
#include "vm.h"
#include "file.h"
#include "wait.h"
#include "kernel.h"

/*
 * exit system call:
 * pass back caller's arg
 */
rexit()
{
	register struct a {
		int	rval;
	} *uap = (struct a *)u.u_ap;

	exit((uap->rval & 0377) << 8);
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit(rv)
{
	register int i;
	register struct proc *p;

	p = u.u_procp;
	p->p_flag &= ~(STRC|SULOCK);
	p->p_sigignore = ~0;
	for (i = 0; i < NSIG; i++)
		u.u_signal[i] = SIG_IGN;
	/*
	 * 2.10 doesn't need to do this and it gets overwritten anyway.
	 * p->p_realtimer.it_value = 0;
	 */
#ifdef CGL_RTP
	/*
	 * if this a "real time" process that is dying
	 * remove the rtpp flag.
	 */
	if (rtpp != NULL && rtpp == p)
		rtpp = NULL;
#endif
	for (i = 0; i <= u.u_lastfile; i++) {
		register struct file *f;

		f = u.u_ofile[i];
		u.u_ofile[i] = NULL;
		u.u_pofile[i] = 0;
		closef(f,1);
	}
	ilock(u.u_cdir);
	iput(u.u_cdir);
	if (u.u_rdir) {
		ilock(u.u_rdir);
		iput(u.u_rdir);
	}
	u.u_rlimit[RLIMIT_FSIZE].rlim_cur = RLIM_INFINITY;
	acct();
	/*
	 * Freeing the user structure and kernel stack
	 * for the current process: have to run a bit longer
	 * using the slots which are about to be freed...
	 */
	if (p->p_flag & SVFORK)
		endvfork();
	else {
		xfree();
		mfree(coremap, p->p_dsize, p->p_daddr);
		mfree(coremap, p->p_ssize, p->p_saddr);
	}
	mfree(coremap, USIZE, p->p_addr);
	if (*p->p_prev = p->p_nxt)		/* off allproc queue */
		p->p_nxt->p_prev = p->p_prev;
	if (p->p_nxt = zombproc)		/* onto zombproc */
		p->p_nxt->p_prev = &p->p_nxt;
	p->p_prev = &zombproc;
	zombproc = p;
#ifdef UCB_METER
	multprog--;
#endif
	p->p_stat = SZOMB;
	noproc = 1;
	{
		register int x;

		i = PIDHASH(p->p_pid);
		x = p - proc;
		if (pidhash[i] == x)
			pidhash[i] = p->p_idhash;
		else {
			for (i = pidhash[i]; i != 0; i = proc[i].p_idhash)
				if (proc[i].p_idhash == x) {
					proc[i].p_idhash = p->p_idhash;
					goto done;
				}
			panic("exit");
		}
	}
	if (p->p_pid == 1) {
		/*
		 * If /etc/init is not found by the icode,
		 * the data size will still be zero when it exits.
		 * Don't panic: we're unlikely to find init after a reboot,
		 * either.
		 */
		if (u.u_dsize == 0) {
			printf("Can't exec /etc/init\n");
			for (;;)
				;
		} else
			panic("init died");
	}
done:
	/*
	 * Overwrite p_alive substructure of proc - better not be anything
	 * important left!
	 */
	p->p_xstat = rv;
	p->p_ru = u.u_ru;
	ruadd(&p->p_ru, &u.u_cru);
	{
		register struct proc *q;
		int doingzomb = 0;

		q = allproc;
again:
		for(; q; q = q->p_nxt)
			if (q->p_pptr == p) {
				q->p_pptr = &proc[1];
				q->p_ppid = 1;
				wakeup((caddr_t)&proc[1]);
				if (q->p_flag&STRC) {
					q->p_flag &= ~STRC;
					psignal(q, SIGKILL);
				} else if (q->p_stat == SSTOP) {
					psignal(q, SIGHUP);
					psignal(q, SIGCONT);
				}
				/*
				 * Protect this process from future
				 * tty signals, clear TSTP/TTIN/TTOU if pending.
				 * 2.10 also sets SDETACH bit.
				 */
				spgrp(q);
			}
		if (!doingzomb) {
			doingzomb = 1;
			q = zombproc;
			goto again;
		}
	}
	psignal(p->p_pptr, SIGCHLD);
	wakeup((caddr_t)p->p_pptr);
	swtch();
}

wait()
{
	struct rusage ru, *rup;

	if ((u.u_ar0[RPS] & PSL_ALLCC) != PSL_ALLCC) {
		u.u_error = wait1(0, (struct rusage *)0);
		return;
	}
	rup = (struct rusage *)u.u_ar0[R1];
	u.u_error = wait1(u.u_ar0[R0], &ru);
	if (u.u_error)
		return;
	if (rup != (struct rusage *)0)
		u.u_error = copyout((caddr_t)&ru, (caddr_t)rup,
		    sizeof (struct rusage));
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait1(options, ru)
	register int options;
	struct rusage *ru;
{
	register f;
	register struct proc *p, *q;

	f = 0;
loop:
	q = u.u_procp;
	/*
	 * 4.X has child links in the proc structure, so they consolidate
	 * these two tests into one loop.  We only have the zombie chain
	 * and the allproc chain, so we check for ZOMBIES first, then for
	 * children that have changed state.  We check for ZOMBIES first
	 * because they are more common, and, as the list is typically small,
	 * a faster check.
	 */
	for (p = zombproc; p;p = p->p_nxt)
		if (p->p_pptr == q) {
			u.u_r.r_val1 = p->p_pid;
			u.u_r.r_val2 = p->p_xstat;
			p->p_xstat = 0;
			if (ru)
				rucvt(ru, &p->p_ru);
			ruadd(&u.u_cru, &p->p_ru);
			p->p_stat = NULL;
			p->p_pid = 0;
			p->p_ppid = 0;
			if (*p->p_prev = p->p_nxt)	/* off zombproc */
				p->p_nxt->p_prev = p->p_prev;
			p->p_nxt = freeproc;		/* onto freeproc */
			freeproc = p;
			p->p_pptr = 0;
			p->p_sig = 0;
			p->p_sigcatch = 0;
			p->p_sigignore = 0;
			p->p_sigmask = 0;
			p->p_pgrp = 0;
			p->p_flag = 0;
			p->p_wchan = 0;
			p->p_cursig = 0;
			return (0);
		}
	for (p = allproc; p;p = p->p_nxt)
		if (p->p_pptr == q) {
			++f;
			if (p->p_stat == SSTOP && (p->p_flag&SWTED)==0 &&
			    (p->p_flag&STRC || options&WUNTRACED)) {
				p->p_flag |= SWTED;
				u.u_r.r_val1 = p->p_pid;
				u.u_r.r_val2 = (p->p_cursig<<8) | WSTOPPED;
				return (0);
			}
		}
	if (f == 0)
		return (ECHILD);
	if (options&WNOHANG) {
		u.u_r.r_val1 = 0;
		return (0);
	}
	if (setjmp(&u.u_qsave)) {
		if ((u.u_sigintr & sigmask(q->p_cursig)) != 0)
			return(EINTR);
		u.u_eosys = RESTARTSYS;
		return (0);
	}
	sleep((caddr_t)q, PWAIT);
	goto loop;
}

/*
 * Notify parent that vfork child is finished with parent's data.  Called
 * during exit/exec(getxfile); must be called before xfree().  The child
 * must be locked in core so it will be in core when the parent runs.
 */
endvfork()
{
	register struct proc *rip, *rpp;

	rpp = u.u_procp;
	rip = rpp->p_pptr;
	rpp->p_flag &= ~SVFORK;
	rpp->p_flag |= SLOCK;
	wakeup((caddr_t)rpp);
	while(!(rpp->p_flag&SVFDONE))
		sleep((caddr_t)rip,PZERO-1);
	/*
	 * The parent has taken back our data+stack, set our sizes to 0.
	 */
	u.u_dsize = rpp->p_dsize = 0;
	u.u_ssize = rpp->p_ssize = 0;
	rpp->p_flag &= ~(SVFDONE | SLOCK);
}
