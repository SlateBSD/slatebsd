/
/

/ a4 -- pdp-11 assembler pass1

rname:
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	$8,r5
	mov	$symbol+8.,r2
	clr	-(r2)
	clr	-(r2)
	clr	-(r2)
	clr	-(r2)
	clr	-(sp)
	clr	-(sp)
	cmp	r0,$'~		/  symbol not for hash table
	bne	1f
	inc	2(sp)
	clrb	ch
1:
	jsr	pc,rch
	movb	chartab(r0),r3
	ble	1f
	add	r3,(sp)
	swab	(sp)
	dec	r5
	blt	1b
	movb	r3,(r2)+
	br	1b
1:
	movb	r0,ch
	mov	(sp)+,r1
	clr	r0
	tst	(sp)+
	beq	1f
	mov	symend,r4
	br	4f
1:
	div	$hshsiz,r0
	ashc	$1,r0
	add	$hshtab,r1
	clr	timesaround
1:
	sub	r0,r1
	cmp	r1,$hshtab
	bhi	2f
	add	$2*hshsiz,r1
	tst	timesaround
	beq	3f

	mov	$8f-9f,-(sp)			/ write(1, ERRMSG, strlen(ERRMSG))
	mov	$9f,-(sp)			/	mov	$1,r0
	mov	$1,-(sp)			/	sys	write; 9f; 8f-9f
	jsr	pc,_write
	add	$6,sp

	jmp	aexit
	.data
timesaround:
	.=.+2
9:	<as: symbol table overflow\n>; 8: .even
	.text
3:
	inc	timesaround
2:
	mov	$symbol,r2
	mov	-(r1),r4
	beq	3f
	cmp	(r2)+,(r4)+
	bne	1b
	cmp	(r2)+,(r4)+
	bne	1b
	cmp	(r2)+,(r4)+
	bne	1b
	cmp	(r2)+,(r4)+
	bne	1b
	br	1f
3:
	mov	symend,r4
	mov	r4,(r1)
4:
	mov	$symbol,r2
	mov	r4,-(sp)
	add	$20,r4
	cmp	r4,0f
	blos	4f
	add	$512.,0f

	mov	r1,-(sp)		/ protect r1 from library
	mov	0f,-(sp)		/ brk(0f)
	jsr	pc,_brk			/	sys	indir; 9f
	tst	(sp)+			/	.data
	mov	(sp)+,r1		/9:	sys	sbreak; 0:end
.data					/	.text
0:	end
.text
4:
	mov	(sp)+,r4
	mov	(r2)+,(r4)+
	mov	(r2)+,(r4)+
	mov	(r2)+,(r4)+
	mov	(r2)+,(r4)+
	clr	(r4)+
	clr	(r4)+
	mov	r4,symend
	sub	$4,r4
1:
	mov	r4,-(sp)
	mov	r4,r3
	sub	$8,r3
	cmp	r3,$usymtab
	blo	1f
	sub	$usymtab,r3
	clr	r2
	div	$3,r2
	mov	r2,r4
	add	$4000,r4		/ user symbol
	br	2f
1:
	sub	$symtab,r3
	clr	r2
	div	$3,r2
	mov	r2,r4
	add	$1000,r4		/ builtin symbol
2:
	jsr	pc,putw
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,r1
	tst	(sp)+
	rts	pc

number:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r5,-(sp)
	clr	r1
	clr	r5
1:
	jsr	pc,rch
	jsr	r5,betwen; '0; '9
		br 1f
	sub	$'0,r0
	mpy	$10.,r5
	add	r0,r5
	als	$3,r1
	add	r0,r1
	br	1b
1:
	cmp	r0,$'b
	beq	1f
	cmp	r0,$'f
	beq	1f
	cmp	r0,$'.
	bne	2f
	mov	r5,r1
	clr	r0
2:
	movb	r0,ch
	mov	r1,r0
	mov	(sp)+,r5
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc
1:
	mov	r0,r3
	mov	r5,r0
	jsr	pc,fbcheck
	add	$141,r0
	cmp	r3,$'b
	beq	1f
	add	$10.,r0
1:
	mov	r0,r4
	mov	(sp)+,r5
	mov	(sp)+,r3
	mov	(sp)+,r2
	add	$2,(sp)
	rts	pc

rch:
	movb	ch,r0
	beq	1f
	clrb	ch
	rts	pc
1:
	dec	inbfcnt
	blt	2f
	movb	*inbfp,r0
	inc	inbfp
	bic	$!177,r0
	beq	1b
	rts	pc
2:
	mov	fin,r0
	beq	3f
	mov	r1,-(sp)		/ protect r1 from library
	mov	$512.,-(sp)		/ read(fin, inbuf, 512)
	mov	$inbuf,-(sp)		/	sys	read; inbuf;512.
	mov	r0,-(sp)		/	bcs	2f
	jsr	pc,_read		/	tst	r0
	add	$6,sp
	mov	(sp)+,r1
	tst	r0
	jmi	2f

	beq	2f
	mov	r0,inbfcnt
	mov	$inbuf,inbfp
	br	1b
2:
	mov	fin,r0
	clr	fin

	mov	r1,-(sp)		/ protect r1 from library
	mov	r0,-(sp)		/ close(r0)
	jsr	pc,_close		/	sys	close
	tst	(sp)+
	mov	(sp)+,r1
3:
	decb	nargs
	bgt	2f
	mov	$'\e,r0
	rts	pc
2:
	tst	ifflg
	beq	2f
	jsr	r5,error; 'i
	jmp	aexit
2:
	mov	curarg,r0
	tst	(r0)+

	mov	r1,-(sp)		/ protect r1 from library
	clr	-(sp)			/ open((r0), O_RDONLY, 0)
	clr	-(sp)			/	mov	(r0),0f
	mov	(r0),-(sp)		/	mov	r0,curarg
	mov	r0,curarg		/	incb	fileflg
	incb	fileflg			/	sys	indir; 9f
	jsr	pc,_open		/	.data
	add	$6,sp			/9:	sys	open; 0:0; 0
	mov	(sp)+,r1		/	.text
	tst	r0			/	bec	2f
	jpl	2f			/	mov	0b,r0
	mov	curarg,r0
	mov	(r0),r0

	jsr	r5,filerr; <?\n>
	jmp	aexit
2:
	mov	r0,fin
	mov	$1,line
	mov	r4,-(sp)
	mov	r1,-(sp)
	mov	$5,r4
	jsr	pc,putw
	mov	*curarg,r1
2:
	movb	(r1)+,r4
	beq	2f
	jsr	pc,putw
	br	2b
2:
	mov	$-1,r4
	jsr	pc,putw
	mov	(sp)+,r1
	mov	(sp)+,r4
	br	1b

