/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)sys.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include <sys/param.h>
#include <sys/fs.h>
#include <sys/dir.h>
#include <sys/inode.h>
#include "saio.h"

/*
 * wfj - mods to trap to explicitly detail why we stopped
 */

int	segflag = 0;


static
openi(n, io)
	ino_t n;
	register struct iob *io;
{
	register struct dinode *dp;
	daddr_t tdp;

	io->i_offset = 0;
	tdp = itod(n);
	io->i_bn = fsbtodb(tdp) + io->i_boff;
	io->i_cc = DEV_BSIZE;
	io->i_ma = io->i_buf;
	devread(io);

	dp = (struct dinode *)io->i_buf;
	dp = &dp[itoo(n)];
	io->i_ino.i_number = n;
	io->i_ino.i_mode = dp->di_mode;
	io->i_ino.i_size = dp->di_size;
	l3tol((caddr_t)io->i_ino.i_addr, (caddr_t)dp->di_addr, NADDR);
}


static
find(path, file)
	register char *path;
	struct iob *file;
{
	register char *q;
	char c;
	int n;

	if (path==NULL || *path=='\0') {
		printf("null path\n");
		return(0);
	}

	openi((ino_t) 2, file);
	while (*path) {
		while (*path == '/')
			path++;
		q = path;
		while(*q != '/' && *q != '\0')
			q++;
		c = *q;
		*q = '\0';

		if ((n=dlook(path, file))!=0) {
			if (c=='\0')
				break;
			openi(n, file);
			*q = c;
			path = q;
			continue;
		} else {
			printf("%s not found\n", path);
			return(0);
		}
		/*NOTREACHED*/
	}
	return(n);
}


static daddr_t
sbmap(io, bn)
	register struct iob *io;
	daddr_t bn;
{
	register i;
	register struct inode *ip;
	int j, sh;
	daddr_t nb, *bap;

	ip = &io->i_ino;;
	if(bn < 0) {
		printf("bn negative\n");
		return((daddr_t)0);
	}

	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if(bn < NADDR-3) {
		i = bn;
		nb = ip->i_addr[i];
		return(nb);
	}

	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	sh = 0;
	nb = 1;
	bn -= NADDR-3;
	for(j=3; j>0; j--) {
		sh += NSHIFT;
		nb <<= NSHIFT;
		if(bn < nb)
			break;
		bn -= nb;
	}
	if(j == 0) {
		printf("bn ovf %D\n", bn);
		return((daddr_t)0);
	}

	/*
	 * fetch the address from the inode
	 */
	nb = ip->i_addr[NADDR-j];
	if(nb == 0) {
		printf("bn void %D\n", bn);
		return((daddr_t)0);
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=3; j++) {
		if (blknos[j] != nb) {
			io->i_bn = fsbtodb(nb) + io->i_boff;
			io->i_ma = b[j];
			io->i_cc = DEV_BSIZE;
			devread(io);
			blknos[j] = nb;
		}
		bap = (daddr_t *)b[j];
		sh -= NSHIFT;
		i = (bn>>sh) & NMASK;
		nb = bap[i];
		if(nb == 0) {
			printf("bn void %D\n", bn);
			return((daddr_t)0);
		}
	}

	return(nb);
}

static ino_t
dlook(s, io)
	char *s;
	register struct iob *io;
{
	register struct v7direct *dp;
	register struct inode *ip;
	daddr_t bn;
	int n, dc;
	daddr_t tbn;

	if (s==NULL || *s=='\0')
		return(0);
	ip = &io->i_ino;
	if ((ip->i_mode&IFMT)!=IFDIR) {
		printf("not a directory\n");
		printf("mode %o, loc %o\n", ip->i_mode, ip);
		return(0);
	}

	n = ip->i_size/sizeof(struct v7direct);

	if (n==0) {
		printf("zero length directory\n");
		return(0);
	}

	dc = DEV_BSIZE;
	bn = (daddr_t)0;
	while(n--) {
		if (++dc >= DEV_BSIZE/sizeof(struct v7direct)) {
			tbn = sbmap(io, bn++);
			io->i_bn = fsbtodb(tbn) + io->i_boff;
			io->i_ma = io->i_buf;
			io->i_cc = DEV_BSIZE;
			devread(io);
			dp = (struct v7direct *)io->i_buf;
			dc = 0;
		}

		if (match(s, dp->d_name))
			return(dp->d_ino);
		dp++;
	}
	return(0);
}

static
match(s1, s2)
	register char *s1, *s2;
{
	register cc;

	cc = MAXNAMLEN;
	while (cc--) {
		if (*s1 != *s2)
			return(0);
		if (*s1++ && *s2++)
			continue; else
			return(1);
	}
	return(1);
}

lseek(fdesc, addr, ptr)
	int fdesc;
	off_t addr;
	int ptr;
{
	register struct iob *io;

	if (ptr != 0) {
		printf("Seek not from beginning of file\n");
		return(-1);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((io = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	io->i_offset = addr;
	io->i_bn = fsbtodb(addr/DEV_BSIZE) + io->i_boff;
	io->i_cc = 0;
	return(0);
}

/* if tape mark encountered, set flag  (in driver) */
int tapemark;

getc(fdesc)
	int fdesc;
{
	register struct iob *io;
	register unsigned char *p;
	register int c;
	int off;
	daddr_t tbn;

	if (fdesc >= 0 && fdesc <= 2)
		return(getchar());
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((io = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	if (io->i_cc <= 0) {
		io->i_bn = fsbtodb(io->i_offset/(off_t)DEV_BSIZE);
		if (io->i_flgs&F_FILE) {
			tbn = sbmap(io, dbtofsb(io->i_bn));
			io->i_bn = fsbtodb(tbn) + io->i_boff;
		}
		io->i_ma = io->i_buf;
		io->i_cc = DEV_BSIZE;
#ifdef DEBUG
		printf("getc: fetch block %D, dev = %d\n", io->i_bn,
				io->i_ino.i_dev);
#endif
		tapemark = 0;
		devread(io);
		if (io->i_flgs&F_FILE) {
			off = io->i_offset % (off_t)DEV_BSIZE;
			if (io->i_offset+(DEV_BSIZE-off) >= io->i_ino.i_size)
				io->i_cc = io->i_ino.i_size - io->i_offset + off;
			io->i_cc -= off;
			if (io->i_cc <= 0)
				return(-1);
		} else {
			off = 0;
			if(tapemark)return(-1);
		}
		io->i_ma = &io->i_buf[off];
	}
	io->i_cc--;
	io->i_offset++;
	c = (int)(*(unsigned char *)io->i_ma++);
	return(c);
}
getw(fdesc)
	int fdesc;
{
	register w, i;
	register unsigned char *cp;
	int val;

	for (i = 0, val = 0, cp = (unsigned char *)&val; i < sizeof(val); i++) {
		w = getc(fdesc);
		if (w < 0) {
			if (i == 0)
				return(-1);
			else
				return(val);
		}
		*cp++ = w;
	}
	return(val);
}

read(fdesc, buf, count)
	int fdesc;
	char *buf;
	int count;
{
	register i;
	register struct iob *file;

	if (fdesc >= 0 & fdesc <= 2) {
		i = count;
		do {
			*buf = getchar();
		} while (--i && *buf++ != '\n');
		return(count - i);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	if ((file->i_flgs&F_READ) == 0)
		return(-1);
	if ((file->i_flgs&F_FILE) == 0) {
		file->i_cc = count;
		file->i_ma = buf;
		i = devread(file);
		file->i_bn += CLSIZE;
		return(i);
	}
	else {
		if (file->i_offset+count > file->i_ino.i_size)
			count = file->i_ino.i_size - file->i_offset;
		if ((i = count) <= 0)
			return(0);
		do {
			*buf++ = getc(fdesc+3);
		} while (--i);
		return(count);
	}
}

write(fdesc, buf, count)
	int fdesc;
	char *buf;
	int count;
{
	register i;
	register struct iob *file;

	if (fdesc >= 0 && fdesc <= 2) {
		i = count;
		while (i--)
			putchar(*buf++);
		return(count);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	if ((file->i_flgs&F_WRITE) == 0)
		return(-1);
	file->i_cc = count;
	file->i_ma = buf;
	i = devwrite(file);
	file->i_bn += CLSIZE;
	return(i);
}

open(str, how)
	char *str;
	int how;
{
	register char *cp;
	int i;
	register struct iob *file;
	register struct devsw *dp;
	int fdesc;
	static first = 1;
	long atol();

	if (first) {
		for (i = 0; i < NFILES; i++)
			iob[i].i_flgs = 0;
		first = 0;
	}

	for (fdesc = 0; fdesc < NFILES; fdesc++)
		if (iob[fdesc].i_flgs == 0)
			goto gotfile;
	_stop("No more file slots");
gotfile:
	(file = &iob[fdesc])->i_flgs |= F_ALLOC;

	for (cp = str; *cp && *cp != '('; cp++)
			;
	if (*cp != '(') {
		printf("Bad device\n");
		file->i_flgs = 0;
		return(-1);
	}
	*cp++ = '\0';
	for (dp = devsw; dp->dv_name; dp++) {
		if (match(str, dp->dv_name))
			goto gotdev;
	}
	printf("Unknown device\n");
	file->i_flgs = 0;
	return(-1);
gotdev:
	*(cp-1) = '(';
	file->i_ino.i_dev = dp-devsw;
	file->i_unit = *cp++ - '0';
	if (file->i_unit < 0 || file->i_unit > 7) {
		printf("Bad unit specifier\n");
		file->i_flgs = 0;
		return(-1);
	}
	if (*cp++ != ',') {
badoff:
		printf("Missing offset specification\n");
		file->i_flgs = 0;
		return(-1);
	}
	file->i_boff = atol(cp);
	for (;;) {
		if (*cp == ')')
			break;
		if (*cp++)
			continue;
		goto badoff;
	}
	devopen(file);
	if (*++cp == '\0') {
		file->i_flgs |= how+1;
		file->i_cc = 0;
		file->i_offset = 0;
		return(fdesc+3);
	}
	if ((i = find(cp, file)) == 0) {
		file->i_flgs = 0;
		return(-1);
	}
	if (how != 0) {
		printf("Can't write files yet.. Sorry\n");
		file->i_flgs = 0;
		return(-1);
	}
	openi(i, file);
	file->i_offset = 0;
	file->i_cc = 0;
	file->i_flgs |= F_FILE | (how+1);
	return(fdesc+3);
}

close(fdesc)
	int fdesc;
{
	struct iob *file;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	if ((file->i_flgs&F_FILE) == 0)
		devclose(file);
	file->i_flgs = 0;
	return(0);
}

exit()
{
	_stop("Exit called");
}

_stop(s)
	char *s;
{
	printf("%s\n", s);
	_rtt();
}

extern char module[];

trap(r1, r0, nps, pc, ps)
	int nps, r1, r0, pc, ps;
{
	printf("Trap in %s,", module);
	switch(nps&7)
	{
	case 0:
		printf("bus error");
		break;
	case 1:
		printf("illegal instruction");
		break;
	case 2:
		printf("bpt/trace");
		break;
	case 3:
		printf("iot");
		break;
	case 4:
		printf("power fail");
		break;
	case 5:
		printf("emt");
		break;
	case 6:
		printf("sys/trap");
		break;
	default:
		printf("weird");
	}
	printf(" at loc %o\n", pc);
	printf("registers: r0=%o, r1=%o, ps=%o, nps=%o\n", r0, r1, ps, nps);
	for (;;)
		;
}
