/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mch_trap.s	1.1 (2.10BSD Berkeley) 2/10/87
 */
#include "DEFS.h"
#include "../machine/mch_iopage.h"
#include "../machine/koverlay.h"
#include "../machine/trap.h"


SPACE(LOCAL, saveps, 2)

/*
 *	jsr	r0,call
 *	jmp	trap_handler
 *
 *	mov	PS,saveps
 *	jsr	r0,call1
 *	jmp	trap_handler
 *
 * Interrupt interface code to C.  Creates an interrupt stack frame, calls
 * the indicated trap_handler.  Call forces the previous mode to be user so
 * that copy in/out and friends use user space if called from interrupt
 * level.  Call1 is simply an alternate entry to call that assumes that the
 * PS has already been changed and the original PS can be found in saveps.
 * Call1 also doesn't increment cnt.v_intr since it's only called from
 * synchronous software traps.
 *
 * The interrupt stack frame - note that values in <pdp/reg.h> must agree:
 *
 *				  reg.h
 *		| ps		|   2	/ ps and pc saved by interrupt
 *		| pc		|   1
 *	u.u_ar0	| r0		|   0
 *		| nps		|	/ from interrupt vector
 *		| ovno		|
 *		| r1		|  -3
 *		| sp		|  -4
 *		| code		|	/ nps & 037
 *	--------------------------------/ above belongs to call, below to csv
 *		| ret		|	/ return address to call
 *	r5	| old r5	|  -7
 *		| ovno		|
 *		| r4		|  -9
 *		| r3		| -10
 *		| r2		| -11
 *	sp	| *empty*	|
 */
call1:
	mov	saveps,-(sp)		/ stuff saved nps into interrupt frame
	SPLLOW				/ ensure low priority (why?)
	br	1f			/ branch around count of interrupts

ASENTRY(call)
	mov	PS, -(sp)		/ stuff nps into interrupt frame
#ifdef UCB_METER
	inc	_cnt+V_INTR		/ cnt.v_intr++
#endif
1:
	mov	__ovno,-(sp)		/ save overlay number,
	mov	r1,-(sp)		/   r1,
	mfpd	sp			/   sp
	mov	6(sp), -(sp)		/ grab nps and calculate
	bic	$!37,(sp)		/   code = nps & 037
	bis	$30000,PS		/ force previous mode = user
	jsr	pc,(r0)			/ call trap_handler

#ifdef UCB_NET
	mov	PS,-(sp)		/ check network to see if it needs
	SPL7				/   servicing
	tst	_netisr			/ net requesting soft interrupt?
	beq	2f
	mov	16.(sp),r0		/ yes, but only if previous spl(ps)
	bic	$!0340,r0		/   is less than splnet (spl2)
	cmp	r0,$NETPRI
	bge	2f
#ifdef UCB_METER
	inc	_cnt+V_SOFT		/ cnt.v_soft++
#endif
	SPLNET				/ set spl = net and service the
	jsr	pc,_netintr		/   request
2:
	mov	(sp)+,PS
#endif

	bit	$30000,8.(sp)		/ previous mode of nps = user?
	beq	4f			/   (note use of supervisor mode will
					/   change this test and mtpd sp below)
	tstb	_runrun			/ yep, is the user's time up?
	beq	3f
	mov	$T_SWITCHTRAP,(sp)	/ yep, set code to T_SWITCHTRAP
	jsr	pc,_trap		/   and give up cpu
3:
	tst	(sp)+			/ toss code, reset user's sp
	mtpd	sp			/   and enter common cleanup code
	br	5f
4:
	cmp	(sp)+,(sp)+		/ trap from kernel: toss code and sp
5:
	mov	(sp)+,r1		/ restore r1

	mov	(sp)+,r0		/ current overlay different from
	cmp	r0,__ovno		/   interrupted overlay?
	beq	6f
	SPL7				/ yes, go non-interruptable (rtt
					/   below restores PS)
	mov	r0,__ovno		/ reset ovno and mapping for
	asl	r0			/   interrupted overlay
	mov	ova(r0), OVLY_PAR
	mov	ovd(r0), OVLY_PDR
6:
	tst	(sp)+			/ toss nps
	mov	(sp)+,r0		/ restore r0
	rtt				/ and return from the trap ...


/*
 * System call interface to C code.  Creates a fake interrupt stack frame and
 * then vectors on to the real syscall handler.  It's not necessary to
 * create a real interrupt stack frame with everything in the world saved
 * since this is a synchronous trap.
 */
ASENTRY(syscall)
	mov	r0, -(sp)
	cmp	-(sp), -(sp)		/ fake __ovno and nps - not needed
	mov	r1, -(sp)
	mfpd	sp			/ grab user's sp for argument addresses
	tst	-(sp)			/ fake code - not needed
	jsr	pc, _syscall		/ call syscall and start cleaning up
	tst	(sp)+
	mtpd	sp			/ reload user sp, r1 and r0
	mov	(sp)+, r1		/   (cret already reloaded the other
	cmp	(sp)+, (sp)+		/   registers)
	mov	(sp)+, r0
	rtt				/ and return from the trap


/*
 * Emt takes emulator traps, which are requests to change the overlay for the
 * current process.  If an invalid emt is received, _trap is called.  Note
 * that choverlay is not called with a trap frame - only r0 and r1 are saved;
 * it's not necessary to save __ovno as this is a synchronous trap.
 */
ASENTRY(emt)
	mov	PS,saveps		/ save PS just in case we need to trap
	bit	$30000,PS		/ if the emt is not from user mode,
	beq	1f			/   or the process isn't overlaid,
	tst	_u+U_OVBASE		/   or the requested overlay number
	beq	1f			/   isn't valid, enter _trap
	cmp	r0,$NOVL
	bhi	1f
	mov	r0,-(sp)		/ everything's cool, save r0 and r1
	mov	r1,-(sp)		/   so they don't get trashed
	mov	r0,_u+U_CUROV		/ u.u_curov = r0
	mov	$RO,-(sp)		/ map the overlay in read only
#ifdef UCB_METER
	inc	_cnt+V_OVLY		/ cnt.v_ovly++
#endif
#ifdef UCB_RUSAGE
	add	$1,_u+U_RU+RU_OVLY+2	/ u.u_ru.ru_ovly++
	adc	_u+U_RU+RU_OVLY
#endif
	jsr	pc,_choverl		/ and get choverlay to bring the overlay in
	tst	(sp)+			/ toss choverlay's paramter,
	mov	(sp)+,r1		/   restore r0 and r1,
	mov	(sp)+,r0
	rtt				/   and return from the trap
1:
	jsr 	r0, call1; jmp	_trap	/ invalid emt
	/*NOTREACHED*/


/*
 * We branch here when we take a trap but find that the variable nofault is
 * set indicating that someone else is interested in taking the trap.
 */
_nofault:
	mov	$1,SSR0			/ re-enable memory management
	mov	nofault,(sp)		/   relocation, fiddle with the
	rtt				/   machine trap frame and boogie


/*
 * Traps without specialized catchers get vectored here.  We re-enable memory
 * management because the fault that caused this trap may have been a memory
 * management fault which sometimes causes the contents of the memory managment
 * registers to be frozen so error recovery can be attempted.
 */
ASENTRY(trap)
	mov	PS,saveps		/ save PS for call1
trap1:
	tst	nofault			/ if someone's already got this trap
	bne	_nofault		/   scoped out, give it to them
trap2:
	/*
	 * save current values of memory management registers in case we
	 * want to back up the instruction that failed
	 */
	mov	SSR0,ssr
#ifndef KERN_NONSEP
	mov	SSR1,ssr+2
#endif !KERN_NONSEP
	mov	SSR2,ssr+4
	mov	$1,SSR0			/ re-enable relocation
	jsr	r0, call1; jmp _trap	/ and let call take us in to _trap ...
	/*NOTREACHED*/


/*
 * Bus error.  Typically a word operation on an odd address or an attempt to
 * access a nonexistent I/O page location.  Buserr acts exactly like trap for
 * the most part, and in fact, most of the time it simply enters trap at
 * various points to let it handle the trap.
 *
 * Buserr will generate a "kernel red stack violation" if the trap came from
 * kernel mode, no fault trap was set, and the current sp is zero.  This
 * condition occurs when an abort is generated when trying to push the ps and
 * pc onto the stack to take a trap.  The cpu responds by loading the sp with
 * 4 and then taking a buserr trap.  Needless to say this is a very serious
 * condition ...
 */
ASENTRY(buserr)
	mov	PS,saveps		/ save PS in case we need to trap
	bit	$30000,PS		/ if previous mode != kernel, or
	bne	trap1			/   fault trap is set, or
	tst	nofault			/   sp != 0, do standard trap
	bne	_nofault		/   processing
	tst	sp
	bne	trap2

	tst	_panicstr		/ ok, we're in trouble; already
	beq	1f			/   paniced?
	br	.			/ yes, just sit tight, don't overwrite
					/   anything
1:
	mov	$intstk+[INTSTK\/2],sp	/ Find a piece of stack so we can panic.
	mov	$redstak,-(sp)		/ (try to leave the intstk intact)
	jsr	pc, _panic		/ and call panic with a red stack
					/   violation
	/*NOTREACHED*/

STRING(LOCAL, redstak, <kernel red stack violation\0>)


#ifdef NONFP
/*
 * Fast illegal-instruction trap routine for use with simulated floating
 * point.  All of the work is done here if SIGILL is caught, otherwise
 * trap is called.  The floating point simulator should really be in the
 * kernal, we just didn't have the time.  Note that anyone using this is
 * crazy.  Floating point processors are now less than $200.
 */
ASENTRY(instrap)
	mov	PS,saveps		/ save PS in case we need to trap
	tst	nofault			/ if somebody already has the trap,
	bne	_nofault		/   give it to them
	bit	$30000,PS		/ if trap isn't from user mode,
	beq	3f			/   enter _trap

	/*
	 * We're going to have to do some grunging around to determine if
	 * SIGILL is being caught.
	 *
	 * struct proc *p = u.u_procp;
	 * if ((p->p_sigcatch & sigmask(SIGILL))
	 *   && !(p->p_sigblock & sigmask(SIGILL)))
	 *	trap();
	 */
	mov	r0,-(sp)		/ save r0 and r1 for the duration
	mov	r1,-(sp)		/   we may need to call grow() ...

#ifdef SIGILL > 16
#	define	SI_BIT	$1\<[SIGILL-16]
#	define	SI_MOFF	0
#else
#	define	SI_BIT	$1\<[SIGILL-1]
#	define	SI_MOFF	2
#endif

	mov	_u+U_PROCP,r0		/ r0 = u.u_procp
	bit	SI_BIT,P_SIGCATCH+SI_MOFF(r0)
	beq	2f			/ SIGILL not being caught
	bit	SI_BIT,P_SIGMASK+SI_MOFF(r0)
	bne	2f			/ SIGILL is being blocked

#undef	SI_BIT
#undef	SI_MOFF

	/*
	 * We need room to put a trap frame (ps and pc) onto the user's
	 * stack, so ...
	 *
	 * sp = user's stack pointer;
	 * if (sp-4 < (caddr_t)-ctob(u.u_ssize) && !grow(sp-4))
	 *	trap();
	 */
	mfpd	sp			/ r0 = user's sp - 4
	mov	(sp)+,r0
	sub	$4,r0
	mov	_u+U_SSIZE,r1		/ r1 = -ctob(u.u_ssize)
	ash	$6,r1
	neg	r1
	cmp	r0,r1
	bhis	1f
	mov	r0,-(sp)		/ sp-4 is below current stack
	mov	r0,-(sp)		/   allocation, save a copy of
	jsr	pc,_grow		/   sp-4 and call grow(sp-4)
	tst	(sp)+			/ (toss parameter)
	mov	(sp)+,r1		/ (grab copy of sp-4)
	tst	r0			/ was grow able to get more stack??
	beq	2f			/ nope, trap out
1:
	/*
	 * The user has enough stack to push a trap frame, so push the user's
	 * old ps and pc, change our return trap frame to send us back to the
	 * floating point simulator (u.u_signal[SIGILL])), trun off tracing
	 * for the simulator to speed things up, and let it happen ...  Note
	 * that since we know there's enough stack, we don't bother setting
	 * nofault ...
	 */
#ifdef UCB_METER
	inc	_cnt+V_TRAP		/ cnt.v_trap++
	inc	_cnt+V_FPSIM		/ cnt.v_fpsim++
#endif
	mov	r1,-(sp)		/ set the user's sp to sp-4
	mtpd	sp
	mov	4(sp),-(sp)		/ we're pushing backwards from the
	mtpd	(r1)			/   new sp upwards, so push trapped
	add	$2,r1			/   pc first, followed by old ps
	mov	6(sp),-(sp)
	mtpd	(r1)
	mov	_u+U_SIGILL,4(sp)	/ set return to fp simulator, and
	bic	$TBIT,6(sp)		/   turn off tracing
	mov	(sp)+,r1		/ restore registers
	mov	(sp)+,r0
	rtt				/ and enter the simulator!
2:
	mov	(sp)+,r1		/ restore registers
	mov	(sp)+,r0
3:
	jsr	r0, call1; jmp _trap	/   generate a SIGILL ...
	/*NOTREACHED*/
#endif NONFP


/*
 * Power fail handling.  On power down, we just set up for the next trap.
 * On power up we attempt an autoboot ...
 */
#define	PVECT	24			/ power fail vector

ASENTRY(powrdown)
	mov	$powrup,PVECT		/ on power up, trap to powrup,
	SPL7				/ and loop at high priority
	br	.

#ifndef KERN_NONSEP
	.data
#endif
	/*
	 * Power back on... wait a bit, then attempt a reboot.  Can't do much
	 * since memory management is off (hence we are in "data" space).
	 */
powrup:
	clr	r0
1:
	sob	r0,1b			/ pause a bit

	mov	$RB_POWRFAIL,r4		/ and try to reboot ...
	mov	_rootdev,r3
	jsr	pc,hardboot
	/*NOTREACHED*/
#ifndef KERN_NONSEP
	.text
#endif


#ifdef UCB_NET
/*
 * Scan net interrupt status register (netisr) for
 * service requests.
 */
ENTRY(netintr)
	mov	r2,-(sp)		/ use r2 for scan of netisr
	mov	KDSD5,-(sp)		/ MAPSAVE();
	mov	KDSA5,-(sp)
/	mov	_seg5+SE_DESC,KDSD5	/ normalseg5();
/	mov	_seg5+SE_ADDR,KDSA5
1:
	SPLHIGH
	mov	_netisr,r2		/ get copy of netisr
	clr	_netisr			/   and clear it, atomically
	SPLNET
#ifdef INET
	bit	$1\<NETISR_IP,r2	/ IP scheduled?
	beq	2f
	jsr	pc,_ipintr
2:
#endif
#ifdef NS
	bit	$1\<NETISR_NS,r2	/ NS scheduled?
	beq	3f
	jsr	pc,_nsintr
3:
#endif
#ifdef IMP
	bit	$1\<NETISR_IMP,r2	/ IMP scheduled?
	beq	4f
	jsr	pc,_impintr
4:
#endif
	bit	$1\<NETISR_RAW,r2	/ RAW scheduled?
	beq	5f			/ checked last since other
	jsr	pc,_rawintr		/ interrupt routines schedule it.
5:
	tst	_netisr			/ see if any new scheduled events
	bne	1b			/   if so, handle them
	mov	(sp)+,KDSA5		/ MAPREST();
	mov	(sp)+,KDSD5
	mov	(sp)+,r2		/ restore saved register,
	rts	pc			/   and return
#endif
