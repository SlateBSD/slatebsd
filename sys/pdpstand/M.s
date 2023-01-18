/
/	SCCS id	@(#)M.s	1.7 (Berkeley)	7/11/83
/
/ Startup code for two-stage bootstrap with support for autoboot.
/ Supports 11/40, 11/45, 11/70, 11/23, 11/23+I/O map (11/24), 11/73
/ and similar machines.

/ Non-UNIX /bin/as instructions.  Most of these instructions are CPU
/ and configuration dependent.

mfpi	= 6500^tst	/ move from previous instruction space
mtpi	= 6600^tst	/ move to previous instruction space
mfpd	= 106500^tst	/ move from previous data space
mtpd	= 106600^tst	/ move to previous data space

stst	= 170300^tst	/ store floating error status registers
ldfps	= 170100^tst	/ load floating processor status register
stfps	= 170200^tst	/ store floating processor status register

spl	= 230		/ set priority level
mfps	= 106700^tst	/ move from PS
mtps	= 106400^tst	/ move to PS
mfpt	= 000007	/ move from processor (get processor model code)

halt	= 0		/ halt cpu
wait	= 1		/ wait for interrupt
iot	= 4		/ perform I/O trap sequence
reset	= 5		/ reset all unibus devices
rtt	= 6		/ return from trap
systrap = 104400	/ trap 0

/  The boot options and device are placed in the last SZFLAGS bytes
/  at the end of core by the kernel if this is an autoboot.
/  The first-stage boot will leave them in registers for us;
/  we clobber possible old flags so that they don't get re-used.
ENDCORE=	160000		/ end of core, mem. management off
SZFLAGS=	6		/ size of boot flags
BOOTOPTS=	2		/ location of options, bytes below ENDCORE
BOOTDEV=	4
CHECKWORD=	6

.globl	_end
.globl	_main,_ubmapset
	jmp	start

/
/ trap vectors
/
	trap;340	/ bus error -- grok!
	trap;341	/ illegal instruction
	trap;342	/ BPT
	trap;343	/ IOT
	trap;344	/ POWER FAIL
	trap;345	/ EMT
tvec:
	start;346	/ TRAP
.=400^.

start:
	reset
	mov	$340,PS
	mov	$_end+512.,sp

/save boot options, if present
	mov	r4,_bootopts
	mov	r3,_bootdev
	mov	r2,_checkword
/clobber any boot flags left in memory
	clr	ENDCORE-BOOTOPTS
	clr	ENDCORE-BOOTDEV
	clr	ENDCORE-CHECKWORD
/
/ determine what kind of cpu we are running on
/ first, check switches. if they are 40, 24, 44, 45, 70, or 73
/ set appropriately
/
	clrb	_sep_id
	clrb	_ubmap
	clrb	_haveCSW
	mov	$2f,nofault	/ check if we even have switches!
	tst	*$SWREG
	clr	nofault		/ apparently we do
	incb	_haveCSW
	mov	$40.,r0
	cmp	*$SWREG,$40
	beq	gotcha
	cmp	*$SWREG,$24
	bne	1f
	mov	$24.,r0
	incb	_ubmap
	jbr	gotcha
1:
	cmp	*$SWREG,$45
	bne	1f
	mov	$45.,r0
	incb	_sep_id
	jbr	gotcha
1:
	cmp	*$SWREG,$70
	bne	2f
	mov	$70.,r0
	incb	_sep_id
	incb	_ubmap
	jbr	gotcha
/
/ if we can't find out from switches,
/ explore and see what we find
/
2:
	mov	$40.,r0		/ assume 11/40
	mov	$2f,nofault
	mov	*$KDSA6,r1	/ then we have sep i/d 45, 70, 73
	incb	_sep_id
	mov	$45.,r0
	mov	$3f,nofault
	mov	*$UBMAP,r1	/ then we have a unibus map. 44 or 70
	incb	_ubmap
	mov	$70.,r0
	mov	$1f,nofault
	mfpt			/ if mfpt instruction exists, this is
	mov	$44.,r0		/   a 44
	br	1f
3:				/ br here with sep i/d, no ubmap. 45 or 73.
	mov	$1f,nofault
	mov	*$MSCR,r1	/ if no MSCR, 45 !
	mov	$73.,r0		/ sep i/d, no ubmap, MSCR. 73!
	br	1f
2:				/ br here if no sep. I/D. 23, 24, 34, 40, 60
	mov	$1f,nofault
	mov	*$UBMAP,r1	/ then we have a unibus map
	incb	_ubmap
	mov	$24.,r0		/ unibus map, no sep. I/D = 24
1:	clr	nofault
gotcha:
	mov	r0,_cputype

/
/	Set kernel I space registers to physical 0 and I/O page
/
	clr	r1
	mov	$77406, r2
	mov	$KISA0, r3
	mov	$KISD0, r4
	jsr	pc, setseg
	mov	$IO, -(r3)


/ Set user I space registers to physical N*64kb and I/O page.  This is
/ where boot will copy itself to.  Boot is very simple minded about its
/ I/O addressing.  To compute physical memory addresses for I/O devices,
/ it simply hands off an address within itself as the low word and
/ ``segflag'' as the high order word.  This has the immediate consequence
/ that boot *MUST* be located on a 64Kb boundry.
/
/ Also, several constraints force us to keep boot in the bootom 248Kb of
/ memory (UNIBUS mapping being a primary contender.)  If boot is ever
/ fixed so that it can be located on a non-64Kb boundry this constraint
/ will still be present.
/
/ All told, unless boot's method of managing its I/O addressing and
/ physical addressing is completely reworked, 3*64Kb is probably the
/ highest we'll ever see boot relocated.  This means that the maximum size
/ of any program boot can load is 192Kb.  That size includes text, data
/ and bss.  This is a problem for the current (9/87) networking kernels
/ which have the networking code and data space incorporated as part of
/ the main line kernel.  Which only adds more impetus to get the networking
/ put into supervisor space ...

N	= 3			/ 3*64Kb = 192Kb

	mov	$N*64.*16., r1	/ N*64 * btoc(1024)
	mov	$UISA0, r3
	mov	$UISD0, r4
	jsr	pc, setseg
	mov	$IO, -(r3)

/
/	If 11/40 class processor, only need set the I space registers
/
	tstb	_sep_id
	jeq	1f

/
/	Set kernel D space registers to physical 0 and I/O page
/
	clr	r1
	mov	$KDSA0, r3
	mov	$KDSD0, r4
	jsr	pc, setseg
	mov	$IO, -(r3)

/
/	Set user D space registers to physical N*64kb and I/O page
/
	mov	$N*64.*16., r1	/ N*64 * btoc(1024)
	mov	$UDSA0, r3
	mov	$UDSD0, r4
	jsr	pc, setseg
	mov	$IO, -(r3)

1:
/ enable map
	clrb	_ksep
	tstb	_ubmap
	beq	2f
	jsr	pc,_ubmapset	/ 24, 44, 70 -> ubmap
	tstb	_sep_id
	bne	3f
	mov	$60,SSR3	/ 24 -> !I/D
	br	1f
3:
	mov	$65,SSR3	/ 44, 70 -> ubmap, I/D
	incb	_ksep
	mov	$3,MSCR		/ 44, 70: $1: disable cache parity traps
				/ 44:     $2: ignored
				/ 70:     $2: disable UNIBUS traps
	br	1f
2:
	tstb	_sep_id		/ 23, 34, 40, 45, 60, 73 -> no ubmap
	beq	1f
	mov	$25,SSR3	/ 45, 73 -> no ubmap, I/D; maybe 22-bit
	incb	_ksep
1:
	mov	$30340,PS
	inc	SSR0


/ copy program to user I space
	mov	$_end,r0
	asr	r0
	clr	r1
1:
	mov	(r1),-(sp)
	mtpi	(r1)+
	sob	r0,1b


/ continue execution in user space copy
	tstb	_sep_id
	bne	1f
	clr	*$KISA6
	br	2f
1:
	clr	*$KDSA6
2:	mov	$140340,-(sp)
	mov	$user,-(sp)
	rtt
user:
/ clear bss
	mov	$_edata,r0
	mov	$_end,r1
	sub	r0,r1
	inc	r1
	clc
	ror	r1
1:
	clr	(r0)+
	sob	r1,1b
	mov	$_end+512.,sp
	mov	sp,r5

	.globl	_segflag
	mov	$N,_segflag

	jsr	pc,_main
	mov	_cputype,r0
	mov	_bootopts,r4
	mov	r4,r2
	com	r2		/ checkword
	systrap

	br	user

setseg:
	mov	$8,r0
1:
	mov	r1,(r3)+
	add	$200,r1
	mov	r2,(r4)+
	sob	r0,1b
	rts	pc

.globl	_setseg
_setseg:
	mov	2(sp),r1
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	$77406,r2
	mov	$KISA0,r3
	mov	$KISD0,r4
	jsr	pc,setseg
	tstb	_ksep
	bne	1f
	mov	$IO,-(r3)
1:
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

.globl	_setnosep
_setnosep:
	bic	$4,SSR3	/ turn off kernel i/d sep
	clrb	_ksep
	rts pc

.globl	_setsep
_setsep:
	bis	$4,SSR3	/ turn on kernel i/d sep (if not already)
	movb	$1,_ksep
	rts pc

/ clrseg(addr,count)
.globl	_clrseg
_clrseg:
	mov	4(sp),r0
	beq	2f
	asr	r0
	bic	$!77777,r0
	mov	2(sp),r1
1:
	clr	-(sp)
	mtpi	(r1)+
	sob	r0,1b
2:
	rts	pc

/ mtpd(word,addr)
.globl	_mtpd
_mtpd:
	mov	4(sp),r0
	mov	2(sp),-(sp)
	mtpd	(r0)+
	rts	pc

/ mtpi(word,addr)
.globl	_mtpi
_mtpi:
	mov	4(sp),r0
	mov	2(sp),-(sp)
	mtpi	(r0)+
	rts	pc

.globl	__rtt
__rtt:
	halt

.globl	_trap

trap:
	mov	*$PS,-(sp)
	mov	r0,-(sp)
	mov	r1,-(sp)
	tst	nofault
	bne	3f
	jsr	pc,_trap
	mov	(sp)+,r1
	mov	(sp)+,r0
	tst	(sp)+
	rtt
3:	mov	(sp)+,r1
	mov	(sp)+,r0
	tst	(sp)+
	mov	nofault,(sp)
	rtt

PS	= 177776
SSR0	= 177572
SSR1	= 177574
SSR2	= 177576
SSR3	= 172516
KISA0	= 172340
KISA1	= 172342
KISA6	= 172354
KISA7	= 172356
KISD0	= 172300
KISD7	= 172316
KDSA0	= 172360
KDSA6	= 172374
KDSA7	= 172376
KDSD0	= 172320
KDSD5	= 172332
SISA0	= 172240
SISA1	= 172242
SISD0	= 172200
SISD1	= 172202
UISA0	= 177640
UISD0	= 177600
UDSA0	= 177660
UDSD0	= 177620
MSCR	= 177746	/ 11/44/60/70 memory system cache control register
IO	= 177600
SWREG	= 177570
UBMAP	= 170200

.data
.globl	_cputype
.globl	_ksep, _sep_id, _ubmap, _haveCSW
.globl	_bootopts, _bootdev, _checkword

nofault:	.=.+2	/ where to go on predicted trap
_cputype:	.=.+2	/ cpu type (currently 40,45 or 70)
_sep_id:	.=.+1	/ 1 if we have separate I and D
_ubmap:		.=.+1	/ 1 if we have a unibus map
_haveCSW:	.=.+1	/ 1 if we have a console switch register
_ksep:		.=.+1	/ 1 if kernel mode has sep I/D enabled
_bootopts:	.=.+2	/ flags if an autoboot
_bootdev:	.=.+2	/ device to get unix from, if not RB_ASKNAME
_checkword:	.=.+2	/ saved r2, complement of bootopts if an autoboot
