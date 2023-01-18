#
/*
 *
 *      UNIX debugger - common definitions
 *
 */



/*      Layout of a.out file (fsym):
 *
 *      header of 8 words       magic number 405, 407, 410, 411, 430, 431
 *                              text size       )
 *                              data size       ) in bytes but even
 *                              bss size        )
 *                              symbol table size
 *                              entry point
 *                              {unused}
 *                              flag set if no relocation
 *      overlay header (if type 430, 431),
 *              NOVL words long:        maximum overlay segement size   )
 *                              overlay 1 size                  )
 *                              overlay 2 size                  )
 *                              overlay 3 size                  )
 *                              overlay 4 size                  ) size
 *                              overlay 5 size                  ) in
 *                              overlay 6 size                  ) even
 *                              overlay 7 size                  ) bytes
 *                              ...
 *                              overlay NOVL size
 *
 *      header:         0
 *      text:           16 (16+2+(2*NOVL)=ovlheader if type 430,431)
 *      overlays (if 430, 431 only):
 *                      16+ovlheader+textsize
 *      overlaysize:    ov1+ov2+...+ovNOVL
 *      data:           16+textsize (+overlaysize+ovlheader if type 431, 431)
 *      relocation:     16+textsize+datasize (should not be present in 430,431)
 *      symbol table:   16+2*(textsize+datasize) or 16+textsize+datasize
 *      (if 430, 431):  16+ovlheader+(textsize+overlaysize+datasize)
 *
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sgtty.h>
#include "mac.h"
#include "mode.h"


/*
 * Internal variables ---
 *  They are addressed by name. (e.g. (`0'-`9', `a'-`b'))
 *  thus there can only be 36 of them.
 */

#define VARB    11
#define VARC    12      /* current overlay number */
#define VARD    13
#define VARE    14
#define VARM    22
#define VARO    24      /* overlay text segment addition */
#define VARS    28
#define VART    29


#define RD      0
#define WT      1
#define NSP     0
#define ISP     1
#define DSP     2
#define STAR    4
#define STARCOM 0200
#define DSYM    7
#define ISYM    2
#define ASYM    1
#define NSYM    0
#define ESYM    0177
#define BKPTSET 1
#define BKPTEXEC 2
#define SYMSIZ  100
#define MAXSIG  20

#define BPT     03
#define FD      0200
#define SETTRC  0
#define RDUSER  2
#define RIUSER  1
#define WDUSER  5
#define WIUSER  4
#define RUREGS  3
#define WUREGS  6
#define CONTIN  7
#define SINGLE  9
#define EXIT    8

#ifndef NONFP
#define FROFF   ((INT)&(((U*)0)->u_fps))
#define FRLEN   25
#define FRMAX   6
#endif

#include <pdp/reg.h>
#define NOREG   32767           /* impossible return from getreg() */
#define NREG    9       /* 8 regs + PS from kernel stack */
/*
 * UAR0 is the value used for subprocesses when there is no core file.
 * If it doesn't correspond to reality, use pstat -u on a core file to
 * get uar0, subtract 0140000, and divide by 2 (sizeof int).
 */
#define UAR0    (&corhdr[ctob(USIZE)/sizeof(POS) - 3]) /* default address of r0 (u.u_ar0) */

#define KR0     (0300/2)       /* location of r0 in kernel dump */
#define KR1     (KR0+1)
#define KR2     (KR0+2)
#define KR3     (KR0+3)
#define KR4     (KR0+4)
#define KR5     (KR0+5)
#define KSP     (KR0+6)
#define KA6     (KR0+7)       /* saved ka6 in kernel dump */

#define MAXOFF  255
#define MAXPOS  80
#define MAXLIN  128
#ifndef EOF
#define EOF     0
#endif	EOF
#define EOR     '\n'
#ifndef TB
#define TB      '\t'
#endif	TB
#define QUOTE   0200
#ifndef STRIP
#define STRIP   0177
#endif	STRIP
#ifndef LOBYTE
#define LOBYTE  0377
#endif	LOBYTE
#define EVEN    -2


/* long to ints and back (puns) */
union {
	INT     I[2];
	L_INT   L;
} itolws;

#define leng(a)         ((long)((unsigned)(a)))
#define shorten(a)      ((int)(a))
#define itol(a,b)       (itolws.I[0]=(a), itolws.I[1]=(b), itolws.L)



/* result type declarations */
L_INT           inkdot();
SYMPTR          lookupsym();
SYMPTR          symget();
POS             get();
POS             chkget();
STRING          exform();
L_INT           round();
BKPTR           scanbkpt();
VOID            fault();

typedef struct sgttyb TTY;
TTY     adbtty, usrtty;
#include <setjmp.h>
jmp_buf erradb;
