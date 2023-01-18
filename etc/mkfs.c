char	*sccsid = "@(#)mkfs.c	2.5";

/*
 * Make a file system prototype.
 * usage: mkfs filsys proto/size [ m n ]
 */
#include <sys/param.h>

#ifndef STANDALONE
#include <stdio.h>
#include <a.out.h>
#endif

#include <sys/file.h>
#include <sys/fs.h>
#include <sys/inode.h>
#include <sys/dir.h>
#include <sys/stat.h>

#define	NIPB	(DEV_BSIZE/sizeof(struct dinode))
#define	NDIRECT	(DEV_BSIZE/sizeof(struct v7direct))
#define	LADDR	(NADDR-3)
#define	MAXFN	500

time_t	utime;

#ifndef STANDALONE
FILE 	*fin;
#else
int	fin;
char	module[] = "Mkfs";
#endif

int	fsi;
int	fso;
char	*charp;
char	buf[DEV_BSIZE];

union {
	struct fblk fb;
	char pad1[DEV_BSIZE];
} fbuf;

#ifndef STANDALONE
struct exec head;
#endif

union {
	struct fs fs;
	char pad2[DEV_BSIZE];
} filsys;

char	string[50];
char	*fsys;
char	*proto;
int	f_n	= 10;
int	f_m	= 5;
int	error;
ino_t	ino;

long	getnum();
daddr_t	alloc();

main(argc,argv)
int	argc;
char	**argv;
{
	int f, c;
	long n;

#ifndef STANDALONE
	time(&utime);
	if(argc < 3) {
		printf("usage: mkfs filsys proto/size [ m n ]\n");
		exit(1);
	}
	fsys = argv[1];
	proto = argv[2];
	fso = creat(fsys, 0666);
	if(fso < 0) {
		printf("%s: cannot create\n", fsys);
		exit(1);
	}
	fsi = open(fsys, 0);
	if(fsi < 0) {
		printf("%s: cannot open\n", fsys);
		exit(1);
	}
	fin = fopen(proto, "r");
#else
	{
		char buf[100];
		static char protos[60];
		printf("%s\n",module);

		do {
			printf("file system: ");
			gets(buf);
			fso = open(buf, 1);
			fsi = open(buf, 0);
		} while (fso < 0 || fsi < 0);

		printf("file sys size: ");
		gets(protos);
		proto = protos;
		printf("interleaving factor (m; %d default): ", f_m);
		gets(buf);
		if (buf[0])
			f_m = atoi(buf);
		printf("interleaving modulus (n; %d default): ", f_n);
		gets(buf);
		if (buf[0])
			f_n = atoi(buf);

		if(f_n <= 0 || f_n >= MAXFN)
			f_n = MAXFN;
		if(f_m <= 0 || f_m > f_n)
			f_m = 3;
	}
	fin = NULL;
	argc = 0;
#endif
	if(fin == NULL) {
		n = 0;
		for(f=0; c=proto[f]; f++) {
			if(c<'0' || c>'9') {
				printf("%s: cannot open\n", proto);
				exit(1);
			}
			n = n*10 + (c-'0');
		}
		filsys.fs.fs_fsize = n;
		/*
		 * Minor hack for standalone root and other
		 * small filesystems: reduce ilist size.
		 */
		if (n <= 5000/CLSIZE)
			n = n/50;
		else
			n = n/25;
		if(n <= 0)
			n = 1;
		if(n > 65500/NIPB)
			n = 65500/NIPB;
		filsys.fs.fs_isize = n + 2;
		printf("isize = %D\n", n*NIPB);
		charp = "d--777 0 0 $ ";
		goto f3;
	}

#ifndef STANDALONE
	/*
	 * Get name of boot load program and read onto block 0.
	 * Don't fail if the magic number is wrong since some of the boot
	 * programs have it stripped to save space.  Don't fail if the
	 * program is too large, either, there might be a reason.  Although
	 * I can't think of one off-hand.
	 */
	getstr();
	if ((f = open(string,O_RDONLY)) < 0)
		perror(string);
	else {
		struct stat	sbuf;

		read(f,(char *)&head,sizeof(struct exec));
		if (head.a_magic != A_MAGIC1) {
			lseek(f, 0L, L_SET);
			printf("mkfs: assuming boot is already stripped, magic number is 0%o, not 0%o.\n",head.a_magic,A_MAGIC1);
			if (!fstat(f,&sbuf) && sbuf.st_size > DEV_BSIZE)
				printf("mkfs: boot too large at %ld; max is %d.\n",sbuf.st_size,DEV_BSIZE);
		}
		else if ((c = head.a_text + head.a_data) > DEV_BSIZE)
			printf("mkfs: boot too large at %d; max is %d.\n",c,DEV_BSIZE);
		read(f,buf,DEV_BSIZE);
		wtfs((long)0,buf);
		close(f);
	}

	/*
	 * get total disk size and inode block size
	 */
	filsys.fs.fs_fsize = getnum();
	n = getnum();
	n /= NIPB;
	filsys.fs.fs_isize = n + 3;

#endif
f3:
	if(argc >= 5) {
		f_m = atoi(argv[3]);
		f_n = atoi(argv[4]);
		if(f_n <= 0 || f_n >= MAXFN)
			f_n = MAXFN;
		if(f_m <= 0 || f_m > f_n)
			f_m = 3;
	}
	filsys.fs.fs_step = f_m;
	filsys.fs.fs_cyl = f_n;
	printf("m/n = %d %d\n", f_m, f_n);
	if(filsys.fs.fs_isize >= filsys.fs.fs_fsize) {
		printf("%ld/%ld: bad ratio\n", filsys.fs.fs_fsize, filsys.fs.fs_isize-2);
		exit(1);
	}
	filsys.fs.fs_tfree = 0;
	filsys.fs.fs_tinode = 0;
	for(c=0; c<DEV_BSIZE; c++)
		buf[c] = 0;
	for(n=2; n!=filsys.fs.fs_isize; n++) {
		wtfs(n, buf);
		filsys.fs.fs_tinode += NIPB;
	}
	ino = 0;

	bflist();

	cfile((struct inode *)0, 0);

	filsys.fs.fs_time = utime;
	wtfs((long)1, (char *)&filsys.fs);
	exit(error);
}

cfile(par, reclevel)
struct inode *par;
{
	struct inode in;
	int dbc, ibc;
	char db[DEV_BSIZE];
	daddr_t ib[NINDIR];
	int i, f, c;

	/*
	 * get mode, uid and gid
	 */

	getstr();
	in.i_mode = gmode(string[0], "-bcd", IFREG, IFBLK, IFCHR, IFDIR);
	in.i_mode |= gmode(string[1], "-u", 0, ISUID, 0, 0);
	in.i_mode |= gmode(string[2], "-g", 0, ISGID, 0, 0);
	for(i=3; i<6; i++) {
		c = string[i];
		if(c<'0' || c>'7') {
			printf("%c/%s: bad octal mode digit\n", c, string);
			error = 1;
			c = 0;
		}
		in.i_mode |= (c-'0')<<(15-3*i);
	}
	in.i_uid = getnum();
	in.i_gid = getnum();

	/*
	 * general initialization prior to
	 * switching on format
	 */

	ino++;
	in.i_number = ino;
	for(i=0; i<DEV_BSIZE; i++)
		db[i] = 0;
	for(i=0; i<NINDIR; i++)
		ib[i] = (daddr_t)0;
	in.i_nlink = 1;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_addr[i] = (daddr_t)0;
	if(par == (struct inode *)0) {
		par = &in;
		in.i_nlink--;
	}
	dbc = 0;
	ibc = 0;
	switch(in.i_mode&IFMT) {

	case IFREG:
		/*
		 * regular file
		 * contents is a file name
		 */

		getstr();
		f = open(string, 0);
		if(f < 0) {
			printf("%s: cannot open\n", string);
			error = 1;
			break;
		}
		while((i=read(f, db, DEV_BSIZE)) > 0) {
			in.i_size += i;
			newblk(&dbc, db, &ibc, ib);
		}
		close(f);
		break;

	case IFBLK:
	case IFCHR:
		/*
		 * special file
		 * content is maj/min types
		 */

		i = getnum() & 0377;
		f = getnum() & 0377;
		in.i_addr[0] = (i<<8) | f;
		break;

	case IFDIR:
		/*
		 * directory
		 * put in extra links
		 * call recursively until
		 * name of "$" found
		 */

		par->i_nlink++;
		in.i_nlink++;
		entry(in.i_number, ".", &dbc, db, &ibc, ib);
		entry(par->i_number, "..", &dbc, db, &ibc, ib);
		in.i_size = 2*sizeof(struct v7direct);
		for(;;) {
			getstr();
			if(string[0]=='$' && string[1]=='\0')
				break;
			entry(ino+1, string, &dbc, db, &ibc, ib);
			in.i_size += sizeof(struct v7direct);
			cfile(&in, reclevel + 1);
		}
		break;
	}
	if (reclevel == 0) {
		entry(ino+1, "lost+found", &dbc, db, &ibc, ib);
		in.i_size += sizeof(struct v7direct);
		mklost(&in);
	}
	if(dbc != 0)
		newblk(&dbc, db, &ibc, ib);
	iput(&in, &ibc, ib);
}

/*ARGSUSED*/
/*VARARGS3*/
gmode(c, s, m0, m1, m2, m3)
char c, *s;
{
	int i;

	for(i=0; s[i]; i++)
		if(c == s[i])
			return((&m0)[i]);
	printf("%c/%s: bad mode\n", c, string);
	error = 1;
	return(0);
}

long
getnum()
{
	int i, c;
	long n;

	getstr();
	n = 0;
	i = 0;
	for(i=0; c=string[i]; i++) {
		if(c<'0' || c>'9') {
			printf("%s: bad number\n", string);
			error = 1;
			return((long)0);
		}
		n = n*10 + (c-'0');
	}
	return(n);
}

getstr()
{
	int i, c;

loop:
	switch(c=getch()) {

	case ' ':
	case '\t':
	case '\n':
		goto loop;

	case '\0':
		printf("EOF\n");
		exit(1);

	case ':':
		while(getch() != '\n');
		goto loop;

	}
	i = 0;

	do {
		string[i++] = c;
		c = getch();
	}
#ifdef	STANDALONE
	while(c!=' '&&c!='\t'&&c!='\n'&&c!='\0');
#else
	while(c!=' '&&c!='\t'&&c!='\n'&&c!='\0' && c != EOF);
#endif
	string[i] = '\0';
}

rdfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fsi, bno*DEV_BSIZE, 0);
	n = read(fsi, bf, DEV_BSIZE);
	if(n != DEV_BSIZE) {
		printf("read error: %ld\n", bno);
		exit(1);
	}
}

wtfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fso, bno*DEV_BSIZE, 0);
	n = write(fso, bf, DEV_BSIZE);
	if(n != DEV_BSIZE) {
		printf("write error: %D\n", bno);
		exit(1);
	}
}

daddr_t
alloc()
{
	int i;
	daddr_t bno;

	filsys.fs.fs_tfree--;
	bno = filsys.fs.fs_free[--filsys.fs.fs_nfree];
	if(bno == 0) {
		printf("out of free space\n");
		exit(1);
	}
	if(filsys.fs.fs_nfree <= 0) {
		rdfs(bno, (char *)&fbuf);
		filsys.fs.fs_nfree = fbuf.fb.df_nfree;
		for(i=0; i<NICFREE; i++)
			filsys.fs.fs_free[i] = fbuf.fb.df_free[i];
	}
	return(bno);
}

bfree(bno)
daddr_t bno;
{
	int i;

	if (bno != 0)
		filsys.fs.fs_tfree++;
	if(filsys.fs.fs_nfree >= NICFREE) {
		fbuf.fb.df_nfree = filsys.fs.fs_nfree;
		for(i=0; i<NICFREE; i++)
			fbuf.fb.df_free[i] = filsys.fs.fs_free[i];
		wtfs(bno, (char *)&fbuf);
		filsys.fs.fs_nfree = 0;
	}
	filsys.fs.fs_free[filsys.fs.fs_nfree++] = bno;
}

entry(inum, str, adbc, db, aibc, ib)
ino_t inum;
char *str;
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	struct v7direct *dp;
	int i;

	dp = (struct v7direct *)db;
	dp += *adbc;
	(*adbc)++;
	dp->d_ino = inum;
	for(i=0; i<MAXNAMLEN; i++)
		dp->d_name[i] = 0;
	for(i=0; i<MAXNAMLEN; i++)
		if((dp->d_name[i] = str[i]) == 0)
			break;
	if(*adbc >= NDIRECT)
		newblk(adbc, db, aibc, ib);
}

newblk(adbc, db, aibc, ib)
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	int i;
	daddr_t bno;

	bno = alloc();
	wtfs(bno, db);
	for(i=0; i<DEV_BSIZE; i++)
		db[i] = 0;
	*adbc = 0;
	ib[*aibc] = bno;
	(*aibc)++;
	if(*aibc >= NINDIR) {
		printf("indirect block full\n");
		error = 1;
		*aibc = 0;
	}
}

getch()
{

#ifndef STANDALONE
	if(charp)
#endif
		return(*charp++);
#ifndef STANDALONE
	return(getc(fin));
#endif
}

bflist()
{
	struct inode in;
	daddr_t ib[NINDIR];
	int ibc;
	char flg[MAXFN];
	int adr[MAXFN];
	int i, j;
	daddr_t f, d;

	for(i=0; i<f_n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<f_n; j++) {
		while(flg[i])
			i = (i+1)%f_n;
		adr[j] = i+1;
		flg[i]++;
		i = (i+f_m)%f_n;
	}

	ino++;
	in.i_number = ino;
	in.i_mode = IFREG;
	in.i_uid = 0;
	in.i_gid = 0;
	in.i_nlink = 0;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_addr[i] = (daddr_t)0;

	for(i=0; i<NINDIR; i++)
		ib[i] = (daddr_t)0;
	ibc = 0;
	bfree((daddr_t)0);
	d = filsys.fs.fs_fsize-1;
	while(d%f_n)
		d++;
	for(; d > 0; d -= f_n)
	for(i=0; i<f_n; i++) {
		f = d - adr[i];
		if(f < filsys.fs.fs_fsize && f >= filsys.fs.fs_isize)
			if(badblk(f)) {
				if(ibc >= NINDIR) {
					printf("too many bad blocks\n");
					error = 1;
					ibc = 0;
				}
				ib[ibc] = f;
				ibc++;
			} else
				bfree(f);
	}
	iput(&in, &ibc, ib);
}

iput(ip, aibc, ib)
struct inode *ip;
int *aibc;
daddr_t *ib;
{
	struct dinode *dp;
	daddr_t d;
	int i;

	filsys.fs.fs_tinode--;
	d = itod(ip->i_number);
	if(d >= filsys.fs.fs_isize) {
		if(error == 0)
			printf("ilist too small\n");
		error = 1;
		return;
	}
	rdfs(d, buf);
	dp = (struct dinode *)buf;
	dp += itoo(ip->i_number);

	dp->di_mode = ip->i_mode;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_atime = utime;
	dp->di_mtime = utime;
	dp->di_ctime = utime;

	switch(ip->i_mode&IFMT) {

	case IFDIR:
	case IFREG:
		for(i=0; i<*aibc; i++) {
			if(i >= LADDR)
				break;
			ip->i_addr[i] = ib[i];
		}
		if(*aibc >= LADDR) {
			ip->i_addr[LADDR] = alloc();
			for(i=0; i<NINDIR-LADDR; i++) {
				ib[i] = ib[i+LADDR];
				ib[i+LADDR] = (daddr_t)0;
			}
			wtfs(ip->i_addr[LADDR], (char *)ib);
		}

	case IFBLK:
	case IFCHR:
		ltol3(dp->di_addr, ip->i_addr, NADDR);
		break;

	default:
		printf("bad mode %o\n", ip->i_mode);
		exit(1);
	}
	wtfs(d, buf);
}

/*ARGSUSED*/
badblk(bno)
daddr_t bno;
{
	return(0);
}

mklost(par)
struct inode *par;
{
	struct inode in;
	int dbc, ibc;
	char db[DEV_BSIZE];
	daddr_t ib[NINDIR];
	int i;

	in.i_mode = IFDIR | ISVTX | 0777;
	in.i_uid = 0;
	in.i_gid = 0;
	in.i_number = ++ino;
	for (i = 0; i < DEV_BSIZE; i++)
		db[i] = 0;
	for (i = 0; i < NINDIR; i++)
		ib[i] = (daddr_t) 0;
	for (i = 0; i < NADDR; i++)
		in.i_addr[i] = (daddr_t)0;
	dbc = 0;
	ibc = 0;
	in.i_nlink = 2;
	/*
	 * blocks 0, ..., NADDR - 4
	 * are direct blocks
	 */
	in.i_size = (off_t) (DEV_BSIZE * (NADDR - 4 + 1));
	par->i_nlink++;
	entry(in.i_number, ".", &dbc, db, &ibc, ib);
	entry(par->i_number, "..", &dbc, db, &ibc, ib);
	for (i = 0; i < NADDR - 4 + 1; i++)
		newblk(&dbc, db, &ibc, ib);
	iput(&in, &ibc, ib);
}
