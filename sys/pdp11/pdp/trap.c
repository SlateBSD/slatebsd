/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)trap.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/psl.h"
#include "../machine/reg.h"
#include "../machine/seg.h"
#include "../machine/trap.h"
#include "../machine/iopage.h"

#include "systm.h"
#include "user.h"
#include "proc.h"
#include "vm.h"

#ifdef DIAGNOSTIC
extern int hasmap;
static int savhasmap;
#endif

/*
 * Offsets of the user's registers relative to
 * the saved r0.  See sys/reg.h.
 */
char regloc[] = {
	R0, R1, R2, R3, R4, R5, R6, R7, RPS
};

#ifndef NONFP
/*
 * Translation table to convert the FP-11 FECs (Floating Exception Codes) to
 * the various FPE_... codes defined in <signal.h>.  On the VAX these come
 * free because those *are* the codes the VAX FP hardware generates.
 */
static int	pdpfec[16] = {
	FPE_CRAZY,		/*  0: not a legal FEC code */
	FPE_CRAZY,		/*  1: not a legal FEC code */
	FPE_OPCODE_TRAP,	/*  2: bad floating point op code */
	FPE_CRAZY,		/*  3: not a legal FEC code */
	FPE_FLTDIV_TRAP,	/*  4: floating devide by zero */
	FPE_CRAZY,		/*  5: not a legal FEC code */
	FPE_DECOVF_TRAP,	/*  6: floating to integer conversion error */
	FPE_CRAZY,		/*  7: not a legal FEC code */
	FPE_FLTOVF_TRAP,	/*  8: floating overflow */
	FPE_CRAZY,		/*  9: not a legal FEC code */
	FPE_CRAZY,		/* 10: not a legal FEC code */
	FPE_FLTUND_TRAP,	/* 11: floating underflow */
	FPE_OPERAND_TRAP,	/* 12: bad floating point operand */
	FPE_CRAZY,		/* 13: not a legal FEC code */
	FPE_MAINT_TRAP,		/* 14: maintenance trap */
	FPE_CRAZY,		/* 15: not a legal FEC code */
};
#endif

/*
 * Called from mch.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the hardware and software during the trap processing.
 * Their order is dictated by the hardware and the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * Dev is the kind of trap that occurred.
 */
/*ARGSUSED*/
trap(dev, sp, r1, ov, nps, r0, pc, ps)
	dev_t dev;
	caddr_t sp, pc;
	int r1, ov, nps, r0, ps;
{
	extern int _ovno;
	static int once_thru = 0;
	register int i;
	register struct proc *p;
	time_t syst;
	mapinfo kernelmap;

#ifdef UCB_METER
	cnt.v_trap++;
#endif
	/*
	 * In order to stop the system from destroying
	 * kernel data space any further, allow only one
	 * fatal trap. After once_thru is set, any
	 * futher traps will be handled by looping in place.
	 */
	if (once_thru) {
		(void)splhigh();
		for(;;);
	}

	if (USERMODE(ps))
		dev |= USER;
	else
		/* guarantee normal kernel mapping */
		savemap(kernelmap);
	syst = u.u_ru.ru_stime;
	p = u.u_procp;
#ifndef NONFP
	u.u_fpsaved = 0;
#endif
	u.u_ar0 = &r0;
	switch(minor(dev)) {

	/*
	 * Trap not expected.  Usually a kernel mode bus error.  The numbers
	 * printed are used to find the hardware PS/PC as follows.  (all
	 * numbers in octal 18 bits)
	 *	address_of_saved_ps =
	 *		(ka6*0100) + aps - 0140000;
	 *	address_of_saved_pc =
	 *		address_of_saved_ps - 2;
	 */
	default:
		once_thru = 1;
#ifdef DIAGNOSTIC
		/*
		 * Clear hasmap if we attempt to sync the fs.
		 * Else it might fail if we faulted with a mapped
		 * buffer.
		 */
		savhasmap = hasmap;
		hasmap = 0;
#endif
		i = splhigh();
		printf("ka6 = %o\n", *ka6);
		printf("aps = %o\n", &ps);
		printf("pc = %o, ps = %o\n", pc, ps);
		printf("__ovno = %d\n", ov);
#if PDP11 == 44 || PDP11 == 70 || PDP11 == GENERIC
		if ((cputype == 70) || (cputype == 44))
			printf("cpuerr = %o\n", *CPUERR);
#endif
		printf("trap type %o\n", dev);
		splx(i);
		panic("trap");

	case T_BUSFLT + USER:
		i = SIGBUS;
		break;

	/*
	 * If illegal instructions are not being caught and the offending
	 * instruction is a SETD, the trap is ignored.  This is because C
	 * produces a SETD at the beginning of every program which will
	 * trap on CPUs without an FP-11.
	 */
#define	SETD	0170011		/* SETD instruction */
	case T_INSTRAP + USER:
		if(fuiword((caddr_t)(pc-2)) == SETD && u.u_signal[SIGILL] == 0)
			goto out;
		i = SIGILL;
		u.u_code = ILL_RESAD_FAULT;	/* it's simplest to lie */
		break;

	case T_BPTTRAP + USER:
		i = SIGTRAP;
		ps &= ~PSL_T;
		break;

	case T_IOTTRAP + USER:
		i = SIGIOT;
		break;

	case T_EMTTRAP + USER:
		i = SIGEMT;
		break;

#ifndef NONFP
	/*
	 * Since the floating exception is an imprecise trap, a user
	 * generated trap may actually come from kernel mode.  In this
	 * case, a signal is sent to the current process to be picked
	 * up later.
	 */
	case T_ARITHTRAP:
		stst(&u.u_fperr);	/* save error code and address */
		u.u_code = pdpfec[(unsigned)u.u_fperr.f_fec & 0xf];
		psignal(p, SIGFPE);
		runrun++;
		restormap(kernelmap);
		return;

	case T_ARITHTRAP + USER:
		i = SIGFPE;
		stst(&u.u_fperr);
		u.u_code = pdpfec[(unsigned)u.u_fperr.f_fec & 0xf];
		break;
#endif

	/*
	 * If the user SP is below the stack segment, grow the stack
	 * automatically.  This relies on the ability of the hardware
	 * to restart a half executed instruction.  On the 11/40 this
	 * is not the case and the routine backup/mch.s may fail.
	 * The classic example is on the instruction
	 *	cmp	-(sp),-(sp)
	 */
	case T_SEGFLT + USER:
		{
			caddr_t osp;

			osp = sp;
			if (backup(u.u_ar0) == 0)
				if (!u.u_onstack && grow((u_int)osp))
					goto out;
			i = SIGSEGV;
			break;
		}

#if PDP11 == 44 || PDP11 == 70 || PDP11 == GENERIC
	/*
	 * The code here is a half-hearted attempt to do something with
	 * all of the PDP11 parity registers.  In fact, there is little
	 * that can be done.
	 */
	case T_PARITYFLT:
	case T_PARITYFLT + USER:
		printf("parity\n");
		if ((cputype == 70) || (cputype == 44)) {
			for(i = 0; i < 4; i++)
				printf("%o ", MEMERRLO[i]);
			printf("\n");
			MEMERRLO[2] = -1;
			if (dev & USER) {
				i = SIGBUS;
				break;
			}
		}
		panic("parity");
#endif

	/*
	 * Allow process switch
	 */
	case T_SWITCHTRAP + USER:
		goto out;

	/*
	 * Whenever possible, locations 0-2 specify this style trap, since
	 * DEC hardware often generates spurious traps through location 0.
	 * This is a symptom of hardware problems and may represent a real
	 * interrupt that got sent to the wrong place.  Watch out for hangs
	 * on disk completion if this message appears.  Uninitialized
	 * interrupt vectors may be set to trap here also.
	 */
	case T_ZEROTRAP:
	case T_ZEROTRAP + USER:
		printf("Trap to 0: ");
		/*FALL THROUGH*/
	case T_RANDOMTRAP:		/* generated by autoconfig */
	case T_RANDOMTRAP + USER:
		printf("Random interrupt ignored\n");
		if (!(dev & USER))
			restormap(kernelmap);
		return;
	}

	/*
	 * If there is a trap from user mode and it is caught, send the
	 * signal now.  This prevents user-mode exceptions from being
	 * delayed by other signals, and in addition is more efficient in
	 * the case of SIGILL and floating-point simulation.
	 */
	{
		long mask = sigmask(i);

		if ((mask & p->p_sigcatch) && !(mask & p->p_sigmask)
		    && !(p->p_flag & STRC)) {
			p->p_sig &= ~mask;	/* just in case ... */
			p->p_cursig = i;
			psig();
		}
		else
			psignal(p, i);
	}
out:
	if (p->p_cursig || ISSIG(p))
		psig();
	curpri = setpri(p);
	if (runrun) {
		setrq(u.u_procp);
#ifdef UCB_RUSAGE
		u.u_ru.ru_nivcsw++;
#endif
		swtch();
	}
	if (u.u_prof.pr_scale)
		addupc(pc, &u.u_prof, (int) (u.u_ru.ru_stime - syst));
#ifndef NONFP
	if (u.u_fpsaved)
		restfp(&u.u_fps);
#endif
}

/*
 * Called from mch.s when a system call occurs.  dev, ov and nps are
 * unused, but are necessitated by the values of the R[0-7] ... constants
 * in sys/reg.h (which, in turn, are defined by trap's stack frame).
 */
/*ARGSUSED*/
syscall(dev, sp, r1, ov, nps, r0, pc, ps)
	dev_t dev;
	caddr_t sp, pc;
	int r1, ov, nps, r0, ps;
{
	extern int nsysent;
	register int code;
	register struct sysent *callp;
	time_t syst;
	register caddr_t opc;	/* original pc for restarting syscalls */

#ifdef UCB_METER
	cnt.v_syscall++;
#endif

	if (!USERMODE(ps))
		panic("syscall");
	syst = u.u_ru.ru_stime;
#ifndef NONFP
	u.u_fpsaved = 0;
#endif
	u.u_ar0 = &r0;
	u.u_error = 0;
	opc = pc - 2;			/* opc now points at syscall */
	code = fuiword((caddr_t)opc) & 0377;	/* bottom 8 bits are index */
	if (code >= nsysent)
		callp = &sysent[0];	/* indir (illegal) */
	else
		callp = &sysent[code];
	if (callp->sy_narg)
		copyin(sp+2, (caddr_t)u.u_arg, callp->sy_narg*NBPW);
	u.u_r.r_val1 = 0;
	u.u_r.r_val2 = r1;
	if (setjmp(&u.u_qsave)) {
		if (u.u_error == 0 && u.u_eosys != RESTARTSYS)
			u.u_error = EINTR;
	} else {
		u.u_eosys = NORMALRETURN;
		(*callp->sy_call)();
#ifdef DIAGNOSTIC
		if (hasmap)
			panic("hasmap");
#endif
	}
	if (u.u_eosys == NORMALRETURN) {
		if (u.u_error) {
			ps |= PSL_C;
			r0 = u.u_error;
		} else {
			ps &= ~PSL_C;
			r0 = u.u_r.r_val1;
			r1 = u.u_r.r_val2;
		}
	} else if (u.u_eosys == RESTARTSYS)
		pc = opc;	/* back up pc to restart syscall */
	/* else if (u.u_eosys == JUSTRETURN) */
		/* nothing to do */
	if (u.u_procp->p_cursig || ISSIG(u.u_procp))
		psig();
	curpri = setpri(u.u_procp);
	if (runrun) {
		setrq(u.u_procp);
#ifdef UCB_RUSAGE
		u.u_ru.ru_nivcsw++;
#endif
		swtch();
	}
	if (u.u_prof.pr_scale)
		addupc(pc, &u.u_prof, (int)(u.u_ru.ru_stime - syst));
#ifndef NONFP
	if (u.u_fpsaved)
		restfp(&u.u_fps);
#endif
}

/*
 * nonexistent system call-- signal process (may want to handle it)
 * flag error if process won't see signal immediately
 * Q: should we do that all the time ??
 */
nosys()
{
	if (u.u_signal[SIGSYS] == SIG_IGN || u.u_signal[SIGSYS] == SIG_HOLD)
		u.u_error = EINVAL;
	psignal(u.u_procp, SIGSYS);
}
