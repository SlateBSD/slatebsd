/
/

/ PDP-11 assembler pass 0

.data
.globl _environ				/ for the standard library
_environ: 0
.text

.globl	_main
_main:
	jmp	start
go:
	jsr	pc,assem

	mov	$512.,-(sp)		/ write(pof, outbuf, 512)
	mov	$outbuf,-(sp)		/	movb	pof,r0
	mov	pof,-(sp)		/	sys	write; outbuf; 512.
	jsr	pc,_write		/	jes	wrterr
	add	$6,sp
	tst	r0
	jmi	wrterr

	mov	pof,-(sp)		/ close(pof)
	jsr	pc,_close		/	movb	pof,r0
					/	sys	close
	mov	fbfil,(sp)		/ close(fbfil)
	jsr	pc,_close		/	movb	fbfil,r0
	tst	(sp)+			/	sys	close

	tstb	errflg
	bne	aexit
	jsr	r5,fcreat; a.tmp3

	mov	r0,-(sp)		/ r0 = fcreat(a.tmp3)
	mov	symend,-(sp)		/ write(r0, usymtab, symend-usymtab)
	sub	$usymtab,(sp)		/	mov	r0,r1
	mov	$usymtab,-(sp)		/	mov	symend,0f
	mov	r0,-(sp)		/	sub	$usymtab,0f
	jsr	pc,_write		/	sys	indir; 9f
	tst	r0			/	jes	wrterr
	jmi	wrterr			/	.data
	add	$6,sp			/9:	sys	write; usymtab; 0:..
					/	.text
					/	mov	r1,r0
	jsr	pc,_close		/ close(r0)
	tst	(sp)+			/	sys	close

	clr	-(sp)			/ execl("/lib/as2", "as2", globfl, ovflag,
	mov	$a.tmp3,-(sp)		/   "-o", outfp, a.tmp1, a.tmp2, a.tmp3, 0)
	mov	$a.tmp2,-(sp)		/	sys	exec; fpass2; 1f
	mov	$a.tmp1,-(sp)		/	mov	$fpass2,r0
	mov	outfp,-(sp)		/	jsr	r5,filerr; "?\n
	mov	$3f,-(sp)		/ { "-o" }
	mov	$ovflag,-(sp)
	mov	$globfl,-(sp)
	mov	$2f,-(sp)		/ { "as2" }
	mov	$1f,-(sp)		/ { "/lib/as2" }
	jsr	pc,_execl

	mov	$1f,r0			/ { "/lib/as2" }
	jsr	r5,filerr; "?\n

aexit:
	mov	$a.tmp1,-(sp)		/ unlink(a.tmp1)
	jsr	pc,_unlink		/	sys	unlink; a.tmp1
	mov	$a.tmp2,(sp)		/ unlink(a.tmp2)
	jsr	pc,_unlink		/	sys	unlink; a.tmp2
	mov	$a.tmp3,(sp)		/ unlink(a.tmp3)
	jsr	pc,_unlink		/	sys	unlink; a.tmp3
	mov	$3,(sp)			/ _exit(3)
	jsr	pc,__exit		/	mov	$3,r0
					/	sys	exit
.data
1:	</lib/as2\0>
2:	<as2\0>
3:	<-o\0>
4:	<a.out\0>
globfl:	unglob=.+1;	<-\0\0>
ovflag:	ovloc=.+1;	<-\0\0>
.even
outfp:	4b				/ { "a.out" }
.text

/ filerr(fname::r0, err::r5)
/	char	*fname, *err;
filerr:
	mov	r1,-(sp)		/ protect r1 from library
	tst	-(sp)			/ write(1, r0, strlen(r0))
	mov	r0,-(sp)
	mov	$1,-(sp)

	clr	r1			/ { strlen(r0) }
1:
	tstb	(r0)+
	beq	2f
	inc	r1
	br	1b
2:
	mov	r1,4(sp)
	jsr	pc,_write
	add	$6,sp

	mov	$1,-(sp)		/ write(1, r5, *(r5+1) ? 2 : 1)
	tstb	1(r5)
	beq	3f
	mov	$2,(sp)
3:
	mov	r5,-(sp)
	mov	$1,-(sp)
	jsr	pc,_write
	add	$6,sp
	mov	(sp)+,r1
	tst	(r5)+
	rts	r5

/ fd = fcreat(fname::r5) { emulates mkstemp }
/	int	fd;
/	char	**fname;
fcreat:
	mov	r1,-(sp)		/ protect r1 from library
	mov	r2,-(sp)
	mov	(r5)+,r2
1:
	mov	$outbuf,-(sp)		/ stat(r5, outbuf)
	mov	r2,-(sp)
	jsr	pc,_stat
	cmp	(sp)+,(sp)+
	tst	r0			/ does the file already exist?
	bmi	3f
	incb	9.(r2)			/ (yes) increment trailing letter
	cmpb	9.(r2),$'z		/ out of temporaries?
	blos	1b
2:
	mov	r2,r0			/ (yes) filerr(*r5, "?\n")
	jsr	r5,filerr; "?\n
	mov	$3,-(sp)		/ _exit(3)
	jsr	pc,__exit
3:
	mov	$0444,-(sp)		/ creat(*r5, 0444)
	mov	r2,-(sp)
	jsr	pc,_creat
	cmp	(sp)+,(sp)+
	tst	r0
	bmi	2b
	mov	(sp)+,r2
	mov	(sp)+,r1
	rts	r5

wrterr:
	mov	$8f-9f, -(sp)		/ write(1, WRTMSG, strlen(WRTERR))
	mov	$9f,-(sp)		/	mov	$1,r0
	mov	$1,-(sp)		/	sys	write; 9f; 9f-8f
	jsr	pc,_write
	add	$6,sp

	incb	errflg
	jbr	aexit

.data
9:	<as: Write error on temp file.\n>; 8: .even
.text
