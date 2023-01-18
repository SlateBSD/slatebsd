/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_proc.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "user.h"
#include "proc.h"
#include "systm.h"

#define bit(a)	(1L<<(a-1))

/*
 * This version is not the 4.XBSD spgrp() for a couple of very good reasons.
 * The 4.2 one didn't work and the 4.3 one requires the child pointers in
 * the proc structure.  This one is pretty simple and loses *big* for several
 * levels of processes since it recursively handles children of children.
 * We don't bother to return the number of processes found as 4.X does since
 * no one ever uses it.  Note, if you have enough processes and unlimited
 * processes per user, someone can crash your system by running you out of
 * kernel stack space.  An alternative method would be to run inferior
 * against every process in the proc structure.  This method is probably
 * faster for most systems.
 */
spgrp(top)
	register struct proc *top;
{
	register struct proc *p;

	top->p_sig &=
		~(sigmask(SIGTSTP)|sigmask(SIGTTIN)|sigmask(SIGTTOU));
	top->p_flag |= SDETACH;
	for (p = allproc; p; p = p->p_nxt)
		if (p->p_pptr == top)
			spgrp(p);
	for (p = zombproc; p; p = p->p_nxt)
		if (p->p_pptr == top)
			spgrp(p);
}

/*
 * Is p an inferior of the current process?
 */
inferior(p)
	register struct proc *p;
{

	for (; p != u.u_procp; p = p->p_pptr)
		if (p->p_ppid == 0)
			return (0);
	return (1);
}

struct proc *
pfind(pid)
	register int pid;
{
	register struct proc *p;

	for (p = &proc[pidhash[PIDHASH(pid)]]; p != &proc[0]; p = &proc[p->p_idhash])
		if (p->p_pid == pid)
			return (p);
	return ((struct proc *)0);
}

/*
 * init the process queues
 */
pqinit()
{
	register struct proc *p;

	/*
	 * most procs are initially on freequeue
	 *	nb: we place them there in their "natural" order.
	 */

	freeproc = NULL;
	for (p = procNPROC; --p > proc; freeproc = p)
		p->p_nxt = freeproc;

	/*
	 * but proc[0] is special ...
	 */

	allproc = p;
	p->p_nxt = NULL;
	p->p_prev = &allproc;

	zombproc = NULL;
}
