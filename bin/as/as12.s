/
/

/ a2 -- pdp-11 assembler pass 1

error:
	incb	errflg
	mov	r0,-(sp)
	mov	r1,-(sp)
	mov	(r5)+,r0
	tst	*curarg
	beq	1f
	mov	r0,-(sp)
	mov	*curarg,r0
	clr	*curarg
	jsr	r5,filerr; '\n
	mov	(sp)+,r0
1:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	line,r3
	movb	r0,1f
	mov	$1f+6,r0
	mov	$4,r1
2:
	clr	r2
	dvd	$10.,r2
	add	$'0,r3
	movb	r3,-(r0)
	mov	r2,r3
	sob	r1,2b

	mov	$7,-(sp)		/ write(1, 1f, 7)
	mov	$1f,-(sp)		/	mov	$1,r0
	mov	$1,-(sp)		/	sys	write; 1f; 7
	jsr	pc,_write
	add	$6,sp

	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,r1
	mov	(sp)+,r0
	rts	r5

	.data
1:	<f xxxx\n>
	.even
	.text

betwen:
	cmp	r0,(r5)+
	blt	1f
	cmp	(r5)+,r0
	blt	2f
1:
	tst	(r5)+
2:
	rts	r5
putw:
	tst	ifflg
	beq	1f
	cmp	r4,$'\n
	bne	2f
1:
	mov	r4,*obufp
	add	$2,obufp
	cmp	obufp,$outbuf+512.
	blo	2f
	mov	$outbuf,obufp

	mov	r1,-(sp)			/ protect r1 from library
	mov	$512.,-(sp)			/ write(pof, outbuf, 512)
	mov	$outbuf,-(sp)			/	movb	pof,r0
	mov	pof,-(sp)			/	sys	write; outbuf; 512.
	jsr	pc,_write			/	jes	wrterr
	add	$6,sp
	mov	(sp)+,r1
	tst	r0
	jmi	wrterr
2:
	rts	pc

