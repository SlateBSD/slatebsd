/*
 *	SCCS id	@(#)raboot.s	1.2 (Berkeley)	2/19/87
 */
#include "localopts.h"

/  The boot options and device are placed in the last SZFLAGS bytes
/  at the end of core for the bootstrap.
ENDCORE=	160000		/ end of core, mem. management off
SZFLAGS=	6		/ size of boot flags
BOOTOPTS=	2		/ location of options, bytes below ENDCORE
BOOTDEV=	4
CHECKWORD=	6

reset= 	5

.globl	_doboot, hardboot
.text
_doboot:
	mov	4(sp),r4	/ boot options
	mov	2(sp),r3	/ boot device

#ifndef	KERN_NONSEP
/  If running separate I/D, need to turn off memory management.
/  Call the routine unmap in low text, after setting up a jump
/  in low data where the PC will be pointing.
.globl	unmap
	mov	$137,*$unmap+2		/ jmp *$hardboot
	mov	$hardboot,*$unmap+4
	jmp	unmap
	/ "return" from unmap will be to hardboot in data
.data
#else
/  Reset to turn off memory management
	reset
#endif

/  On power fail, hardboot is the entry point (map is already off)
/  and the args are in r4, r3.

hardboot:
	mov	r4, ENDCORE-BOOTOPTS
	mov	r3, ENDCORE-BOOTDEV
	com	r4		/ if CHECKWORD == ~bootopts, flags are believed
	mov	r4, ENDCORE-CHECKWORD
1:
	reset

/  The remainder of the code is dependent on the boot device.
/  If you have a bootstrap ROM, just jump to the correct entry.
/  Otherwise, use a BOOT opcode, if available;
/  if necessary, read in block 0 to location 0 "by hand".
/
/ RA bootstrap.
/
/ Note: this is a complex boot, but then MSCP is complex!!!!
/
/ Bootstrap for mscp disk controller tucker@gswd-vms
/

MSCPSIZE =	64.	/ One MSCP command packet is 64bytes long (need 2)

RASEMAP	=	140000	/ RA controller owner semaphore

RAERR =		100000	/ error bit 
RASTEP1 =	04000	/ step1 has started
RAGO =		01	/ start operation, after init
RASTCON	= 	4	/ Setup controller info 
RAONLIN	=	11	/ Put unit on line
RAREAD =	41	/ Read command code
RAWRITE =	42	/ Write command code
RAEND =		200	/ End command code

RACMDI =	4.	/ Command Interrupt
RARSPI =	6.	/ Response Interrupt
RARING =	8.	/ Ring base
RARSPL =	8.	/ Response Command low
RARSPH = 	10.	/ Response Command high
RACMDL =	12.	/ Command to controller low
RACMDH =	14.	/ Command to controller high
RARSPS =	16.	/ Response packet length (location)
RARSPREF =	20.	/ Response reference number
RACMDS =	80.	/ Command packet length (location)
RACMDREF =	84.	/ Command reference number
RAUNIT = 	88.	/ Command packet unit 
RAOPCODE =	92.	/ Command opcode offset
RABYTECT =	96.	/ Transfer byte count
RABUFL =	100.	/ Buffer location (16 bit addressing only)
RABUFH = 	102.	/ Buffer location high 6 bits
RALBNL =	112.	/ Logical block number low
RALBNH = 	114.	/ Logical block number high

raip	= 172150	/ initialization and polling register
rasa	= 172152	/ address and status register

BSIZE	=	512.	/ Size of boot block
/
/ options:
/
unit	= 0		/ # of unit to load boot from

/
/ Clear RA MSCP command area!
/
	mov	$ra,r0
	mov	$BSIZE,r1
	mov	r0,sp
	add	r1,sp
1:
	clr	(r0)+
	sob	r1,1b
/
/ RA initialize controller 
/
	mov	$RASTEP1,r0
	mov	$raip,r1
	clr	(r1)+			/ go through controller init seq.
	mov	$icons,r2
1:
	bit	r0,(r1)
	beq	1b
	mov	(r2)+,(r1)
	asl	r0
	bpl	1b
	mov	$ra+RARSPREF,*$ra+RARSPL / set controller characteristics
	mov	$ra+RACMDREF,*$ra+RACMDL
	mov	$RASTCON,r0
	jsr	pc,racmd
	mov	$unit,*$ra+RAUNIT	/ bring boot unit online
	mov	$RAONLIN,r0
	jsr	pc,racmd

/
/ Read in block zero and jump to it
/
/
/ RA MSCP read block routine.  This is very primative, so don't expect
/ too much from it.  Note that MSCP requires large data communications
/ space at end of ADDROFF for command area.
/
/	Load block zero into memory location zero.
/	BSIZE	->	size of block to read
/ 
/ Tim Tucker, Gould Electronics, August 23rd 1985
/
	clr	r0
	mov	r0,*$ra+RALBNL		/ Put in logical block number
	mov	$BSIZE,*$ra+RABYTECT	/ Put in byte to transfer
	mov	r0,*$ra+RABUFL		/ Put in disk buffer location
	mov	$RAREAD,r0
	jsr	pc,racmd
	jmp	*$0			/ and away we go
/
/ perform MSCP command -> response poll version
/
racmd:
	movb	r0,*$ra+RAOPCODE	/ fill in command type
	mov	$MSCPSIZE,r0
	mov	r0,*$ra+RARSPS		/ give controller struct sizes
	mov	r0,*$ra+RACMDS
	mov	$RASEMAP,r0
	mov	r0,*$ra+RARSPH		/ set mscp semaphores
	mov	r0,*$ra+RACMDH
	mov	raip,r0			/ tap controllers shoulder
	mov	$ra+RACMDI,r0
1:
	tst	(r0)
	beq	1b			/ Wait till command read
	clr	(r0)+			/ Tell controller we saw it, ok.
2:
	tst	(r0)
	beq	2b			/ Wait till response written
	clr	(r0)			/ Tell controller we got it
	rts	pc

icons:	RAERR
	ra+RARING
	0
	RAGO
ra:	0
