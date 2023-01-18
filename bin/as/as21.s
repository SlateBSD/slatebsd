/
/

/ a21 -- pdp-11 assembler pass 2 

.data
.globl _environ				/ for the standard library
_environ: 0
.text

.globl	_main
_main:
	mov	$1,-(sp)		/ signal(SIGINT, SIG_IGN)
	mov	$2,-(sp)		/	sys	signal; 2; 1
	jsr	pc,_signal
	cmp	(sp)+,(sp)+

	ror	r0
	bcs	1f

	mov	$saexit,-(sp)		/ signal(SIGINT, saexit)
	mov	$2,-(sp)		/	sys	signal; 2; saexit
	jsr	pc,_signal
	cmp	(sp)+,(sp)+
1:
	jmp	start

/ set up sizes and origins

go:

/ read in symbol table

	mov	$usymtab,r1
1:
	jsr	pc,getw
	bvs	1f
	add	$14,symsiz		/ count symbols
	jsr	pc,getw
	jsr	pc,getw
	jsr	pc,getw
	jsr	pc,getw
	mov	r4,r0
	bic	$!37,r0
	cmp	r0,$2			/text
	blo	2f
	cmp	r0,$3			/data
	bhi	2f
	add	$31,r4			/mark "estimated"
	mov	r4,(r1)+
	jsr	pc,getw
	mov	r4,(r1)+
	br	3f
2:
	clr	(r1)+
	clr	(r1)+
	jsr	pc,getw
3:
	jsr	pc,setbrk
	br	1b
1:

/ read in f-b definitions

	mov	r1,fbbufp
	mov	fbfil,fin
	clr	ibufc
1:
	jsr	pc,getw
	bvs	1f
	add	$31,r4			/ "estimated"
	mov	r4,(r1)+
	jsr	pc,getw
	mov	r4,(r1)+
	jsr	pc,setbrk
	br	1b
1:
	mov	r1,endtable
	mov	$100000,(r1)+

/ set up input text file; initialize f-b table

	jsr	pc,setup
/ do pass 1

	jsr	pc,assem

/ prepare for pass 2
	cmp	outmod,$777
	beq	1f
	jmp	aexit
1:
	clr	dot
	mov	$2,dotrel
	mov	$..,dotdot
	clr	brtabp

	mov	r1,-(sp)		/ protect r1 from library
	mov	fin,-(sp)		/ close(fin)
	jsr	pc,_close		/	movb	fin,r0
	tst	(sp)+			/	sys	close
	mov	(sp)+,r1

	jsr	r5,ofile; a.tmp1
	mov	r0,fin
	clr	ibufc
	jsr	pc,setup
	inc	passno
	inc	bsssiz
	bic	$1,bsssiz
	mov	txtsiz,r1
	inc	r1
	bic	$1,r1
	mov	r1,txtsiz
	mov	datsiz,r2
	inc	r2
	bic	$1,r2
	mov	r2,datsiz
	mov	r1,r3
	mov	r3,datbase	/ txtsiz
	mov	r3,savdot+2
	add	r2,r3
	mov	r3,bssbase	/ txtsiz+datsiz
	mov	r3,savdot+4
	clr	r0
	asl	r3
	adc	r0
	add	$20,r3
	adc	r0
	mov	r3,symseek+2	/ 2*txtsiz+2*datsiz+20
	mov	r0,symseek
	sub	r2,r3
	sbc	r0
	mov	r3,drelseek+2	/ 2*txtsiz+datsiz
	mov	r0,drelseek
	sub	r1,r3
	sbc	r0
	mov	r3,trelseek+2	/ txtsiz+datsiz+20
	mov	r0,trelseek
	sub	r2,r3
	sbc	r0
	mov	r0,datseek
	mov	r3,datseek+2	/ txtsiz+20
	mov	$usymtab,r1
1:
	jsr	pc,doreloc
	add	$4,r1
	cmp	r1,endtable
	blo	1b
	clr	r0
	clr	r1
	jsr	r5,oset; txtp
	mov	trelseek,r0
	mov	trelseek+2,r1
	jsr	r5,oset; relp
	mov	$8.,r2
	mov	$txtmagic,r1
1:
	mov	(r1)+,r0
	jsr	r5,putw; txtp
	dec	r2
	bne	1b
	jsr	pc,assem

/polish off text and relocation

	jsr	r5,flush; txtp
	jsr	r5,flush; relp

/ append full symbol table

	mov	symf,r0
	mov	r0,fin

	clr	-(sp)			/ lseek(fin, 0L, L_SET)
	clr	-(sp)			/	sys	lseek; 0; 0; 0
	clr	-(sp)
	mov	r0,-(sp)
	jsr	pc,_lseek
	add	$8.,sp

	clr	ibufc
	mov	symseek,r0
	mov	symseek+2,r1
	jsr	r5,oset; txtp
	mov	$usymtab,r1
1:
	jsr	pc,getw
	bvs	1f
	mov	r4,r0
	jsr	r5,putw; txtp
	jsr	pc,getw
	mov	r4,r0
	jsr	r5,putw; txtp
	jsr	pc,getw
	mov	r4,r0
	jsr	r5,putw; txtp
	jsr	pc,getw
	mov	r4,r0
	jsr	r5,putw; txtp
	mov	(r1)+,r0
	jsr	r5,putw; txtp
	mov	(r1)+,r0
	jsr	r5,putw; txtp
	jsr	pc,getw
	jsr	pc,getw
	br	1b
1:
	jsr	r5,flush; txtp
	jmp	aexit

saexit:
	mov	pc,errflg

aexit:
	mov	a.tmp1,-(sp)		/ unlink(a.tmp1)
	jsr	pc,_unlink		/	mov	a.tmp1,0f
					/	sys	unlink; 0:..
	mov	a.tmp2,(sp)		/ unlink(a.tmp2)
	jsr	pc,_unlink		/	mov	a.tmp2,0f
					/	sys	unlink; 0:..
	mov	a.tmp3,(sp)		/ unlink(a.tmp3)
	jsr	pc,_unlink		/	mov	a.tmp3,0f
					/	sys	unlink; 0:..
	tst	errflg
	jne	2f

	clr	(sp)			/ umask(0)
	jsr	pc,_umask		/	sys	umask; 0

	bic	r0,outmod		/ chmod(a.outp2, outmod&umask(0))
	mov	outmod,(sp)		/	bic	r0,outmod
	mov	a.outp2,-(sp)		/	sys	chmod; a.outp2:a.out; outmod: 777
	jsr	pc,_chmod
	tst	(sp)+
.data
a.outp2:	a.out
outmod:		0777
.text

	clr	(sp)
	br	1f
2:
	mov	$2,(sp)
1:
	jsr	pc,__exit		/ _exit(errflg ? 2 : 0)
					/	sys	exit
filerr:
	mov	*(r5),r5
	tst	-(sp)			/ write(1, r5, strlen(r5))
	mov	r5,-(sp)
	mov	$1,-(sp)
	clr	r0
1:
	tstb	(r5)+
	beq	2f
	inc	r0
	br	1b
2:
	mov	r0,4(sp)
	jsr	pc,_write
	add	$6,sp

	mov	$2,-(sp)		/ write(1, "?\n", 2)
	mov	$qnl,-(sp)
	mov	$1,-(sp)
	jsr	pc,_write
	add	$6,sp
	jmp	saexit

doreloc:
	movb	(r1),r0
	bne	1f
	bisb	defund,(r1)
1:
	bic	$!37,r0
	cmp	r0,$5
	bhis	1f
	cmp	r0,$3
	blo	1f
	beq	2f
	add	bssbase,2(r1)
	rts	pc
2:
	add	datbase,2(r1)
1:
	rts	pc

setbrk:
	mov	r1,-(sp)
	add	$20,r1
	cmp	r1,0f
	blo	1f
	add	$512.,0f

	mov	0f,-(sp)		/ brk(0f)
	jsr	pc,_brk			/	sys	indir; 9f
	tst	(sp)+			/	.data
.data					/9:	sys	sbreak; 0: end
0:	end				/	.text
.text
1:
	mov	(sp)+,r1
	rts	pc

setup:
	mov	$curfb,r4
1:
	clr	(r4)+
	cmp	r4,$curfb+40.
	blo	1b
	mov	txtfil,fin
	clr	ibufc
	clr	r4
1:
	jsr	pc,fbadv
	tstb	(r4)+
	cmp	r4,$10.
	blt	1b
	rts	pc

ofile:
	mov	r1,-(sp)		/ protect r1 from library
	clr	-(sp)			/ open(*(r5), O_RDONLY, 0)
	clr	-(sp)			/	mov	*(r5),0f
	mov	*(r5),-(sp)		/	sys	indir; 9f
	jsr	pc,_open		/	.data
	add	$6,sp			/9:	sys	open; 0:..; 0
	mov	(sp)+,r1		/	.text
	tst	r0			/	bes	1f
	bmi	1f

	tst	(r5)+
	rts	r5
1:
	jmp	filerr
