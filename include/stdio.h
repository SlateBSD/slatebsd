/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)stdio.h	5.4 (Berkeley) 3/4/87
 */

# ifndef FILE

/*
 * The stdio macros getc, getchar, putc and putchar use fgetc and fputc
 * unless USE_STDIO_MACROS is defined.  Using fgetc and fputc almost always
 * saves text space and if assembly versions are available can save time as
 * well.  As the macros and routines are completely compatible, there is no
 * harm in mixing objects that do and don't use the macros - which is most
 * easily proven by noting that the C versions of fgetc and fputc use the
 * macros themselves ...  The funny ifndef below is so one can define
 * USE_STDIO_MACROS before including <stdio.h> and not get a `... redefined'
 * message from cpp.
 */
#ifndef USE_STDIO_MACROS
/*#define	USE_STDIO_MACROS	/**/
#endif !USE_STDIO_MACROS

#define	BUFSIZ	1024
extern	struct	_iobuf {
	int	_cnt;
	char	*_ptr;		/* should be unsigned char */
	char	*_base;		/* ditto */
	int	_bufsiz;
	short	_flag;
	char	_file;		/* should be short */
} _iob[];

#define	_IOREAD	01
#define	_IOWRT	02
#define	_IONBF	04
#define	_IOMYBUF	010
#define	_IOEOF	020
#define	_IOERR	040
#define	_IOSTRG	0100
#define	_IOLBF	0200
#define	_IORW	0400
#define	NULL	0
#define	FILE	struct _iobuf
#define	EOF	(-1)

#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])

#ifdef USE_STDIO_MACROS
#ifndef lint
#define	getc(p)		(--(p)->_cnt>=0? (int)(*(unsigned char *)(p)->_ptr++):_filbuf(p))
#endif not lint
#define	getchar()	getc(stdin)
#ifndef lint
#define putc(x, p)	(--(p)->_cnt >= 0 ?\
	(int)(*(unsigned char *)(p)->_ptr++ = (x)) :\
	(((p)->_flag & _IOLBF) && -(p)->_cnt < (p)->_bufsiz ?\
		((*(p)->_ptr = (x)) != '\n' ?\
			(int)(*(unsigned char *)(p)->_ptr++) :\
			_flsbuf(*(unsigned char *)(p)->_ptr, p)) :\
		_flsbuf((unsigned char)(x), p)))
#endif not lint
#define	putchar(x)	putc(x,stdout)
#else !USE_STDIO_MACROS
#define	getc(p)		fgetc(p)
#define	getchar()	fgetc(stdin)
#define	putc(x, p)	fputc(x, p)
#define	putchar(x)	fputc(x, stdout)
#endif USE_STDIO_MACROS

#define	feof(p)		(((p)->_flag&_IOEOF)!=0)
#define	ferror(p)	(((p)->_flag&_IOERR)!=0)
#define	fileno(p)	((p)->_file)
#define	clearerr(p)	((p)->_flag &= ~(_IOERR|_IOEOF))

FILE	*fopen();
FILE	*fdopen();
FILE	*freopen();
FILE	*popen();
long	ftell();
char	*fgets();
char	*gets();
#ifdef vax
char	*sprintf();		/* too painful to do right */
#endif
# endif
