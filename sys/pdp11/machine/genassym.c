/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)genassym.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include <stdio.h>
#include "user.h"
#include "proc.h"
#include "inode.h"
#include "mbuf.h"
#include "buf.h"
#include "errno.h"
#include "reboot.h"
#include "syscall.h"
#include "vm.h"
#include "dz.h"
#include "../net/netisr.h"

struct proc proc[1];		/* satisfy proc.h and inode.h */
struct inode inode[1];
struct buf buf[1];

main()
{
	{
		struct buf *bp = 0;

		printf("#define B_ADDR %o\n",&bp->b_un.b_addr);
		printf("#define B_XMEM %o\n",&bp->b_xmem);
	}

	{
		segm *se = 0;

		printf("#define SE_ADDR %o\n",&se->se_addr);
		printf("#define SE_DESC %o\n",&se->se_desc);
	}

	{
		struct user *u = 0;

		printf("#define U_CUROV %o\n",&u->u_ovdata.uo_curov);
		printf("#define U_OVBASE %o\n",&u->u_ovdata.uo_ovbase);
		printf("#define U_SIGILL %o\n",&(u->u_signal[SIGILL]));
		printf("#define U_RU %o\n",&u->u_ru);
		printf("#define U_PROCP %o\n",&u->u_procp);
		printf("#define U_SSIZE %o\n",&u->u_ssize);
	}

	{
		struct proc *p = 0;

		printf("#define P_SIGCATCH %o\n",&p->p_sigcatch);
		printf("#define P_SIGMASK %o\n",&p->p_sigmask);
	}

#ifdef UCB_NET
	{
		struct mbuf *m = 0;
		struct mbufx *mx = 0;

		printf("#define M_CLICK %o\n",&m->m_click);
		printf("#define M_MBUF %o\n",&mx->m_mbuf);
		printf("#define M_OFF %o\n",&m->m_off);
	}
#endif

	{
		struct k_rusage *ru = 0;

		printf("#define RU_OVLY %o\n",&ru->ru_ovly);
	}

	{
		struct vmrate *vm = 0;

		printf("#define V_TRAP %o\n",&vm->v_trap);
		printf("#define V_INTR %o\n",&vm->v_intr);
		printf("#define V_SOFT %o\n",&vm->v_soft);
		printf("#define V_PDMA %o\n",&vm->v_pdma);
		printf("#define V_OVLY %o\n",&vm->v_ovly);
		printf("#define V_FPSIM %o\n",&vm->v_fpsim);
	}

	printf("#define DEV_BSIZE %d.\n",DEV_BSIZE);
	printf("#define EFAULT %d.\n",EFAULT);
	printf("#define ENOENT %d.\n",ENOENT);
	printf("#define MAXBSIZE %d.\n",MAXBSIZE);
	printf("#define MBMAPSIZE %o\n",MBMAPSIZE);
	printf("#define MBX %o\n",MBX);
	printf("#define NETISR_IMP %d.\n",NETISR_IMP);
	printf("#define NETISR_IP %d.\n",NETISR_IP);
	printf("#define NETISR_NS %d.\n",NETISR_NS);
	printf("#define NETISR_RAW %d.\n",NETISR_RAW);
	printf("#define NOVL %d.\n",NOVL);
	printf("#define RB_POWRFAIL %d.\n",RB_POWRFAIL);
	printf("#define RB_SINGLE %d.\n",RB_SINGLE);
	printf("#define SIGILL %d.\n",SIGILL);
	printf("#define SYS_execv %d.\n",SYS_execv);
	printf("#define SYS_exit %d.\n",SYS_exit);
	printf("#define USIZE %d.\n",USIZE);

	exit(0);
}
