/
/

/ a4 -- pdp-11 assembler pass 2

oset:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	(r5)+,r3
	mov	r1,r2
	bic	$!777,r1
	add	r3,r1
	add	$8,r1
	mov	r1,(r3)+	/ next slot
	mov	r3,r1
	add	$1006,r1
	mov	r1,(r3)+	/ buf max
	mov	r0,(r3)+
	mov	r2,(r3)+	/ seek addr
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	r5

putw:
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	(r5)+,r2
	mov	(r2)+,r1	/ slot
	cmp	r1,(r2)		/ buf max
	bhis	1f
	mov	r0,(r1)+
	mov	r1,-(r2)
	br	2f
1:
	tst	(r2)+
	mov	r0,-(sp)
	jsr	r5,flush1
	mov	(sp)+,r0
	mov	r0,*(r2)+
	add	$2,-(r2)
2:
	mov	(sp)+,r2
	mov	(sp)+,r1
	rts	r5

flush:
	mov	(r5)+,r2
	cmp	(r2)+,(r2)+
flush1:

	clr	-(sp)			/ lseek(fout, (r2)L+, L_SET)
	mov	2(r2),-(sp)		/	mov	(r2)+,0f
	mov	(r2)+,-(sp)		/	mov	(r2)+,r1
	tst	(r2)+			/	mov	r1,0f+2		/ seek address
	mov	fout,-(sp)		/	mov	fout,r0
	jsr	pc,_lseek		/	sys	indir; 9f
	add	$8.,sp			/	.data
					/9:	sys	lseek; 0:..; ..; 0
					/	.text
	cmp	-(sp),-(sp)		/ write(fout, <buf>, <len>)
	bic	$!777,r1
	add	r2,r1		/ write address
	mov	r1,-(sp)		/ { <buf> }
	mov	r2,r0
	bis	$777,-(r2)
	add	$1,(r2)		/ new seek addr
	adc	-(r2)
	cmp	-(r2),-(r2)
	sub	(r2),r1
	neg	r1
	mov	r1,2(sp)	/ count
	mov	r0,(r2)		/ new next slot

	mov	fout,-(sp)
	mov	r1,6(sp)		/ protect r1 from library
	jsr	pc,_write
	add	$6,sp
	mov	(sp)+,r1
	tst	r0
	jmi	wrterr

	rts	r5

wrterr:
	mov	$8f-9f,-(sp)		/ write(1, ERRMSG, strlen(ERRMSG))
	mov	$9f,-(sp)		/	mov	$1,r0
	mov	$1,-(sp)		/	sys	write; 9f; 8f-9f
	jsr	pc,_write
	add	$6,sp

	jmp	saexit
.data
9:	<as: write error on output\n>; 8: .even
.text

readop:
	mov	savop,r4
	beq	1f
	clr	savop
	rts	pc
1:
	jsr	pc,getw1
	cmp	r4,$200
	blo	1f
	cmp	r4,$4000
	blo	2f
	add	$usymtab-4000,r4
	rts	pc
2:
	add	$symtab-1000,r4
1:
	rts	pc

getw:
	mov	savop,r4
	beq	getw1
	clr	savop
	rts	pc
getw1:
	dec	ibufc
	bgt	1f

	mov	r1,-(sp)		/ protect r1 from library
	mov	$512.,-(sp)		/ read(fin, inbuf, 512)
	mov	$inbuf,-(sp)		/	movb	fin,r0
	mov	fin,-(sp)		/	sys	read; inbuf; 512.
	jsr	pc,_read		/	bes	3f
	add	$6,sp
	mov	(sp)+,r1
	tst	r0
	jmi	3f

	asr	r0
	mov	r0,ibufc
	bne	2f
3:
	mov	$4,r4
	sev
	rts	pc
2:
	mov	$inbuf,ibufp
1:
	mov	*ibufp,r4
	add	$2,ibufp
	rts	pc
