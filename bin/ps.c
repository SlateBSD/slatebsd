char	*sccsid	= "%W%";

/*
 *	ps - process status
 *	This is the augmented UCB ps for 2.10BSD PDP-11 Unix (11/82).
 *	It is not very portable, using the phys sys call and
 *	knowing the format of an a.out symbol table.
 *	Examine and print certain things about processes
 *	Usage:  ps [ acgklnrtuwxU# ] [ corefile [ swapfile [ system ] ] ]
 */

#include <sys/param.h>
#include <stdio.h>
#include <pwd.h>
#include <nlist.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <OLD/core.h>
#include <OLD/psout.h>

#define	N_BADMAG(x) \
	(((x).a_magic)!=A_MAGIC1 && ((x).a_magic)!=A_MAGIC2 && \
	((x).a_magic)!=A_MAGIC3 && ((x).a_magic)!=A_MAGIC4 && \
	((x).a_magic)!=A_MAGIC5 && ((x).a_magic)!=A_MAGIC6)

#define	N_TXTOFF(x) \
	((x).a_magic==A_MAGIC5 || (x).a_magic==A_MAGIC6 ? \
	sizeof (struct ovlhdr) + sizeof (struct exec) : sizeof (struct exec))

#define	equal		!strcmp
#define	exists(x)	(stat ((x), &stbuf) == 0)
#define	within(x,y,z)	(((unsigned)(x) >= (y)) && ((unsigned)(x) < (z)))
#define	round(x,y)	((long) ((((long) (x) + (long) (y) - 1L) / (long) (y)) * (long) (y)))

struct	nlist nl[] =	{
	{ "_proc" , 0, 0},
#define	X_PROC		0
	{ "_swplo" , 0, 0},
#define	X_SWPLO		1
	{ "_nproc", 0, 0},
#define X_NPROC		2
	{ "_hz", 0, 0},
#define	X_HZ		3
	0
};
#define	NNAMESIZ	(sizeof nl[0].n_name)

struct	proc *mproc, proc [8];
struct	user u;
struct	stat	stbuf;

int	hz;
int	chkpid	= 0;
int	aflg;	/* -a: all processes, not just mine */
int	cflg;	/* -c: not complete listing of args, just comm. */
int	gflg;	/* -g: complete listing including group headers, etc */
int	kflg;	/* -k: read from core file instead of real memory */
int	lflg;	/* -l: long listing form */
int	nflg;	/* -n: numeric wchans */
int	rflg;	/* -r: raw output in style <psout.h> */
int	uflg;	/* -u: user name */
int	wflg;	/* -w[w]: wide terminal */
int	xflg;	/* -x: ALL processes, even those without ttys */
int	Uflg;	/* -U: update the private list */
char	*tptr, *mytty;
char	*nlistf, *uname;
int	file;
off_t	swplo;
int	nproc;
off_t	tell;
int	nchans;
int	nttys;
int	nsyms;
int	ismem;
#ifndef	PSFILE
char	*psdb	= "/etc/psdatabase";
#else
char	*psdb	= PSFILE;
#endif

/*
 *	Structure for the unix wchan table
 */
typedef struct wchan {
	char	cname [NNAMESIZ];
	unsigned	caddr;
} WCHAN;
WCHAN	*wchanhd;

char	*calloc (), *malloc (), *realloc ();
char	*gettty (), *getptr (), *getchan ();
char	*ttyname ();
int	pscomp ();
int	wchancomp();
off_t	lseek ();

#ifndef	MAXTTYS
#define	MAXTTYS		256
#endif
struct	ttys	{
	char	name[MAXNAMLEN];
	dev_t	ttyd;
} allttys[MAXTTYS];

struct	map	{
	off_t	b1, e1; off_t	f1;
	off_t	b2, e2; off_t	f2;
};
struct	map datmap;

struct	psout *outargs;		/* info for first npr processes */
int	npr;			/* number of processes found so far */
int	twidth;			/* terminal width */
int	cmdstart;		/* starting position for command field */

char	*memf;			/* name of kernel memory file to use */
char	*kmemf;			/* name of physical memory file to use */
char	*swapf;			/* name of swap file to use */
char	*nlistf;		/* name of symbol table file to use */
int	kmem, mem, swap;
char	pbuf[BUFSIZ];
#ifdef	TERMCAP
char	*getenv ();
#endif

main (argc, argv)
char	**argv;
{
	int	uid, euid, puid, nread;
	register	i, j;
	char	*ap,
		*strcpy();
	register	struct	proc	*procp;
#ifdef	TERMCAP
	char	*termp, capbuf [1024];
#endif

#ifdef	TERMCAP
	if ((termp = getenv ("TERM")) != (char *) NULL)
		if (tgetent (capbuf, termp) == 1)
			twidth	= tgetnum ("co");
		else	;
	else
#endif
		twidth	= 80;

	setbuf (stdout, pbuf);
	argc--, argv++;
	if (argc > 0)	{
		ap	= argv [0];
		while (*ap) switch (*ap++)	{
		case '-':
			break;

		case 'a':
			aflg++;
			break;

		case 'c':
			cflg++;
			break;

		case 'g':
			gflg++;
			break;

		case 'k':
			kflg++;
			break;

		case 'l':
			lflg	= 1;
			break;

		case 'n':
			nflg++;
			lflg	= 1;
			break;

		case 'r':
			rflg++;
			break;

		case 't':
			if (*ap)
				tptr = ap;
			else if ((tptr = ttyname(0)) != 0) {
				tptr = strcpy(mytty, tptr);
				if (strncmp(tptr, "/dev/", 5) == 0)
					tptr += 5;
			}
			if (strncmp(tptr, "tty", 3) == 0)
				tptr += 3;
			aflg++;
			gflg++;
			if (tptr && *tptr == '?')
				xflg++;
			while (*ap)
				ap++;
			break;

		case 'u':
			uflg	= 1;
			break;

		case 'U':
			Uflg++;
			break;

		case 'w':
			if (twidth == 80)
				twidth	= 132;
			else	twidth	= BUFSIZ;
			wflg++;
			break;

		case 'x':
			xflg++;
			break;

		default:
			if (!isdigit (ap[-1]))
				break;
			chkpid	= atoi (--ap);
			*ap	= '\0';
			aflg++;
			xflg++;
			break;
		}
	}

	openfiles (argc, argv);
	getkvars (argc, argv);
	if (kflg)
		swplo	= (off_t) 0;
	uid	= getuid ();
	euid	= geteuid ();
	mytty	= ttyname(0);
	if (!strncmp(mytty,"/dev/",5)) mytty += 5;
	if (!strncmp(mytty,"tty",3)) mytty += 3;
	printhdr();
	for (i = 0; i < nproc; i += 8)	{
		j	= nproc - i;
		if (j > 8)
			j	= 8;
		j	*= sizeof (struct proc);
		if ((nread = read (kmem, (char *) proc, j)) != j)	{
			cantread ("proc table", kmemf);
			if (nread == -1)
				break;
			}
		for (j = nread / sizeof (struct proc) - 1; j >= 0; j--)	{
			mproc	= &proc[j];
			procp	= mproc;
			/* skip processes that don't exist */
			if (procp->p_stat == 0)
				continue;
			/* skip those without a tty unless -x */
			if (procp->p_pgrp == 0 && xflg == 0)
				continue;
			/* skip group leaders on a tty unless -g, -x, or -t.. */
			if (!tptr && !gflg && !xflg && procp->p_ppid == 1 && (procp->p_flag & SDETACH) == 0)
				continue;
			/* -g also skips those where **argv is "-" - see savcom */
			puid = procp->p_uid;
			/* skip other peoples processes unless -a or a specific pid */
			if ((uid != puid && euid != puid && aflg == 0) ||
			    (chkpid != 0 && chkpid != procp->p_pid))
				continue;
			if (savcom (puid))
				npr++;
		}
	}
	fixup (npr);
	for (i = 0; i < npr; i++)	{
#ifdef	TERMCAP
		register	cmdwidth	= twidth - cmdstart - 2;
#endif
		register	struct	psout	*a	= &outargs[i];

		if (rflg)	{
			if (write (1, (char *) a, sizeof (struct psout)) != sizeof (struct psout))
				perror ("write");
			continue;
			}
		else	if (lflg)
				lpr (a);
			else	if (uflg)
					upr (a);
				else	spr (a);
#ifdef	TERMCAP
		if (cmdwidth < 0)
			cmdwidth	= 80 - cmdstart - 2;
#endif
		if (a->o_stat == SZOMB)
#ifdef	TERMCAP
			printf ("%.*s", cmdwidth, " <defunct>");
#else
			printf (" <defunct>");
#endif
		else	if (a->o_pid == 0)
#ifdef	TERMCAP
				printf ("%.*s", cmdwidth, " swapper");
#else
				printf (" swapper");
#endif
			else	printf (" %.*s", twidth - cmdstart - 2, cflg ?  a->o_comm : a->o_args);
		putchar ('\n');
		}
	exit (!npr);
}

struct	direct *dbuf;
int	dialbase;

getdev()
{
	register DIR *df;

	if (chdir("/dev") < 0) {
		perror("/dev");
		exit(1);
	}
	dialbase = -1;
	if ((df = opendir(".")) == NULL) {
		fprintf(stderr, "Can't open . in /dev\n");
		exit(1);
	}
	while ((dbuf = readdir(df)) != NULL) 
		maybetty();
	closedir(df);
}

/*
 * Attempt to avoid stats by guessing minor device
 * numbers from tty names.  Console is known,
 * know that r(hp|up|mt) are unlikely as are different mem's,
 * floppy, null, tty, etc.
 */
maybetty()
{
	register char *cp = dbuf->d_name;
	register struct ttys *dp;
	int x;
	struct stat stb;

	switch (cp[0]) {

	case 'c':
		if (!strcmp(cp, "console")) {
			x = 0;
			goto donecand;
		}
		/* cu[la]? are possible!?! don't rule them out */
		break;

	case 'd':
		if (!strcmp(cp, "drum"))
			return;
		break;

	case 'f':
		if (!strcmp(cp, "floppy"))
			return;
		break;

	case 'k':
		cp++;
		if (*cp == 'U')
			cp++;
		goto trymem;

	case 'r':
		cp++;
#define is(a,b) cp[0] == 'a' && cp[1] == 'b'
		if (is(h,p) || is(r,a) || is(u,p) || is(h,k) 
		    || is(r,b) || is(m,t)) {
			cp += 2;
			if (isdigit(*cp) && cp[2] == 0)
				return;
		}
		break;

	case 'm':
trymem:
		if (cp[0] == 'm' && cp[1] == 'e' && cp[2] == 'm' && cp[3] == 0)
			return;
		if (cp[0] == 'm' && cp[1] == 't')
			return;
		break;

	case 'n':
		if (!strcmp(cp, "null"))
			return;
		if (!strncmp(cp, "nrmt", 4))
			return;
		break;

	case 'p':
		if (cp[1] && cp[1] == 't' && cp[2] == 'y')
			return;
		break;

	case 'v':
		if ((cp[1] == 'a' || cp[1] == 'p') && isdigit(cp[2]) &&
		    cp[3] == 0)
			return;
		break;
	}
	cp = dbuf->d_name + dbuf->d_namlen - 1;
	x = 0;
	if (cp[-1] == 'd') {
		if (dialbase == -1) {
			if (stat("ttyd0", &stb) == 0)
				dialbase = stb.st_rdev & 017;
			else
				dialbase = -2;
		}
		if (dialbase == -2)
			x = 0;
		else
			x = 11;
	}
	if (cp > dbuf->d_name && isdigit(cp[-1]) && isdigit(*cp))
		x += 10 * (cp[-1] - ' ') + cp[0] - '0';
	else if (*cp >= 'a' && *cp <= 'f')
		x += 10 + *cp - 'a';
	else if (isdigit(*cp))
		x += *cp - '0';
	else
		x = -1;
donecand:
	if (nttys >= MAXTTYS) {
		fprintf(stderr, "ps: tty table overflow\n");
		exit(1);
	}
	dp = &allttys[nttys++];
	(void)strcpy(dp->name, dbuf->d_name);
	if (Uflg) {
		if (stat(dp->name, &stb) == 0 &&
		   (stb.st_mode&S_IFMT)==S_IFCHR)
			dp->ttyd = x = stb.st_rdev;
		else {
			nttys--;
			return;
		}
	} else
		dp->ttyd = -1;
}

savcom (puid)
{
	char	*tp;
	off_t	addr;
	off_t	daddr, saddr;
	register	struct	psout	*a;
	register	struct	proc	*procp	= mproc;
	register	struct	user	*up	= &u;
	long	txtsiz, datsiz, stksiz;
	int	septxt;

	if (procp->p_flag & SLOAD) {
		addr = ctob ((off_t) procp->p_addr);
		daddr = ctob ((off_t) procp->p_daddr);
		saddr = ctob ((off_t) procp->p_saddr);
		file = mem;
	}
	else {
		addr = (procp->p_addr + swplo) << 9;
		daddr = (procp->p_daddr + swplo) << 9;
		saddr = (procp->p_saddr + swplo) << 9;
		file = swap;
	}
	if (pread (file, (char *) up, sizeof (struct user), addr) != sizeof (struct user))
		return(0);

	txtsiz = ctob (up->u_tsize);	/* set up address maps for user pcs */
	datsiz = ctob (up->u_dsize);
	stksiz = ctob (up->u_ssize);
	septxt = up->u_sep;
	datmap.b1 = (septxt ?  0 : round (txtsiz, TXTRNDSIZ));
	datmap.e1 = datmap.b1 + datsiz;
	datmap.f1 = daddr;
	datmap.f2 = saddr;
	datmap.b2 = stackbas (stksiz);
	datmap.e2 = stacktop (stksiz);
	tp = gettty();
	if ((tptr && strcmp(tptr,tp)) || (strcmp(mytty, tp) && !aflg))
		return(0);
	a = &outargs[npr];		/* saving com starts here */
	a->o_uid = puid;
	a->o_pid = procp->p_pid;
	a->o_flag = procp->p_flag;
	a->o_ppid = procp->p_ppid;
	a->o_cpu  = procp->p_cpu;
	a->o_pri = procp->p_pri;
	a->o_nice = procp->p_nice;
	a->o_addr0 = procp->p_addr;
	a->o_size = ctod(procp->p_dsize + procp->p_ssize + USIZE);
	a->o_wchan = procp->p_wchan;
	a->o_pgrp = procp->p_pgrp;
	strncpy(a->o_tty, tp, 8);
	a->o_ttyd = tp[0] == '?' ?  -1 : up->u_ttyd;
	a->o_stat = procp->p_stat;
	a->o_flag = procp->p_flag;

	if (a->o_stat == SZOMB)
		return(1);
	a->o_utime = up->u_ru.ru_utime;
	a->o_stime = up->u_ru.ru_stime;
	a->o_cutime = up->u_cru.ru_utime;
	a->o_cstime = up->u_cru.ru_stime;
	a->o_sigs = (int)up->u_signal[SIGINT] + (int)up->u_signal[SIGQUIT];
	a->o_uname[0] = 0;
	strncpy(a->o_comm, up->u_comm, 14);

	if (cflg)
		return (1);
	else
		return(getcmd(a,saddr));
}

char *
gettty ()
{
	register int tty_step;
	register char *p;

	if (u.u_ttyp)
		for (tty_step = 0;tty_step < nttys;++tty_step)
			if (allttys[tty_step].ttyd == u.u_ttyd) {
				p = allttys[tty_step].name;
				if (!strncmp(p,"tty",3))
					p += 3;
				return(p);
			}
	return("?");
}

/*
 * fixup figures out everybodys name and sorts into a nice order.
 */
fixup (np)
register	np;
{
	register	i;
	register	struct	passwd	*pw;
	struct	passwd	*getpwent ();
	char	*strcpy();

	if (uflg)	{
		/*
		 * If we want names, traverse the password file. For each
		 * passwd entry, look for it in the processes.
		 * In case of multiple entries in /etc/passwd, we believe
		 * the first one (same thing ls does).
		 */
		while ((pw = getpwent ()) != (struct passwd *) NULL)	{
			for (i = 0; i < np; i++)
				if (outargs[i].o_uid == pw->pw_uid)	{
					if (outargs[i].o_uname[0] == 0)
						strcpy (outargs[i].o_uname, pw->pw_name);
					}
			}
		}

	qsort (outargs, np, sizeof (outargs[0]), pscomp);
}

pscomp (x1, x2)
register	struct	psout	*x1, *x2;
{
	register	c;

	c	= (x1)->o_ttyd - (x2)->o_ttyd;
	if (c == 0)
		c	= (x1)->o_pid - (x2)->o_pid;
	return (c);
}

wchancomp (x1, x2)
register WCHAN	*x1,
		*x2;
{
	if (x1->caddr > x2->caddr)
		return (1);
	else	if (x1->caddr == x2->caddr)
			return (0);
		else	return (-1);
}

char	*
getptr (adr)
char	**adr;
{
	char	*ptr;
	register	char	*p, *pa;
	register	i;

	ptr	= 0;
	pa	= (char *)adr;
	p	= (char *)&ptr;
	for (i = 0; i < sizeof (ptr); i++)
		*p++	= getbyte (pa++);
	return (ptr);
}

getbyte (adr)
register	char	*adr;
{
	register	struct	map	*amap	= &datmap;
	char	b;
	off_t	saddr;

	if (!within (adr, amap->b1, amap->e1))
		if (within (adr, amap->b2, amap->e2))
			saddr	= (unsigned) adr + amap->f2 - amap->b2;
		else	return (0);
	else	saddr	= (unsigned) adr + amap->f1 - amap->b1;
	if (lseek (file, saddr, 0) == (off_t) -1 || read (file, &b, 1) < 1)
		return (0);
	return ((unsigned) b);
}

/*
 * pread is like read, but if it's /dev/mem we use the phys
 * system call for speed.  On systems without phys we have
 * to use regular read.
 */
pread(fd,ptr,nbytes,loc)
int	fd;
char	*ptr;
int	nbytes;
off_t	loc;
{
	if (fd == mem && ismem) {
		if (phys(6,nbytes / 64 + 1,(short)(loc / 64)) >= 0) {
			bcopy(0140000,ptr,nbytes);
			return(nbytes);
		}
	}
	lseek(fd,loc,0);
	return(read(fd,ptr,nbytes));
}

addchan (name, caddr)
char	*name;
unsigned	caddr;
{
	register int	nc = nchans;
	register WCHAN	*wp = wchanhd;

	if (!nc)
		wp = (WCHAN *)malloc(sizeof(WCHAN));
	else
		wp = (WCHAN *)realloc(wp, sizeof(WCHAN) * (nc + 1));
	if (!wp) {
		fputs("Too many symbols\n",stderr);
		exit(1);
	}
	strncpy (wp[nc].cname, name, NNAMESIZ - 1);
	wp[nc].cname[NNAMESIZ-1]	= '\0';
	wp[nc].caddr	= caddr;
	wchanhd = wp;
	nchans++;
}

char	*
getchan (chan)
register	unsigned	chan;
{
	register	i;
	register	char	*prevsym;

	prevsym	= "";
	if (chan)
		for (i = 0; i < nchans; i++)	{
			if (wchanhd[i].caddr > chan)
				return (prevsym);
			prevsym = wchanhd[i].cname;
			}
	return (prevsym);
}

nlist()
{
	register FILE	*nlistf_fp;
	register struct nlist *nnn;
	struct nlist	nbuf;
	struct exec	hbuf;
	int	nllen;
	off_t	sa;

	nllen = sizeof(nl) / sizeof(struct nlist);
	if (!(nlistf_fp = fopen(nlistf,"r")))
		perrexit(nlistf);
	if (fread(&hbuf,sizeof(hbuf),1,nlistf_fp) != 1) {
		fputs("Invalid symbol table\n",stderr);
		exit(1);
	}
	if (N_BADMAG(hbuf)) {
		fprintf(stderr,"%s: not in object file format\n",nlistf);
		exit(1);
	}
	if (hbuf.a_magic == A_MAGIC5 || hbuf.a_magic == A_MAGIC6) {
		register int	novl_step;
		struct ovlhdr	ovlbuf;

		fread((char *)&ovlbuf,1,sizeof(ovlbuf),nlistf_fp);
		for (sa = novl_step = 0;novl_step < NOVL;++novl_step)
			sa += (off_t)(ovlbuf.ov_siz)[novl_step];
		fseek(nlistf_fp,sa,1);
	}
	sa = (off_t)hbuf.a_text + hbuf.a_data;
	if (!(hbuf.a_flag & 01))
		sa *= (off_t)2;
	fseek(nlistf_fp,sa,1);
	nsyms = 0;
	while (fread(&nbuf,sizeof(nbuf),1,nlistf_fp) == 1) {
		register int	flag;

		nsyms++;
		if (nbuf.n_name[0] != '_' )
			continue;
		flag = nbuf.n_type & (N_TYPE | N_EXT);
		if ((nbuf.n_type & N_TYPE) != N_ABS && flag !=  (N_EXT | N_DATA) && flag != (N_EXT | N_BSS))
			continue;
		if (!nflg)
			addchan(nbuf.n_name + 1,(unsigned)(nbuf.n_value));
		if (nllen)
			for (nnn = nl;*nnn->n_name;nnn++)
				if (!strncmp(nnn->n_name,nbuf.n_name,NNAMESIZ)) {
					nnn->n_value = nbuf.n_value;
					nnn->n_type = nbuf.n_type;
					--nllen;
					break;
				}
	}
	if (nsyms == 0) {
		fprintf(stderr,"%s: no symbol table\n",nlistf);
		exit(1);
	}
	fclose(nlistf_fp);
	if (!nflg)
		qsort(wchanhd,nchans,sizeof(WCHAN),wchancomp);
}

perrexit(msg)
char	*msg;
{
	perror(msg);
	exit(1);
}

writepsdb()
{
	register FILE	*fp;
	int	nllen;

	setuid(getuid());
	if (!(fp = fopen (psdb, "w")))
		perrexit(psdb);
	else
		chmod(psdb,0644);
	nllen = sizeof(nl) / sizeof(struct nlist);
	fwrite(nlistf,strlen(nlistf) + 1,1,fp);
	fwrite((char *)&nllen,sizeof(nllen),1,fp);
	fwrite((char *)&nttys,sizeof(nttys),1,fp);
	fwrite((char *)&nchans,sizeof(nchans),1,fp);
	fwrite((char *)nl,sizeof(struct nlist),nllen,fp);
	fwrite((char *)allttys,sizeof(struct ttys),nttys,fp);
	fwrite((char *)wchanhd,sizeof(WCHAN),nchans,fp);
	fclose(fp);
}

char	*
readpsdb ()
{
	register int	i;
	register FILE	*fp;
	register WCHAN	*ccc;
	static char	unamebuf[BUFSIZ];
	char	*p = unamebuf;
	int	nllen;

	if ((fp = fopen (psdb, "r")) == NULL)
		perrexit (psdb);

	while ((*p= getc (fp)) != '\0')
		p++;
	fread (&nllen, sizeof nllen, 1, fp);
	fread (&nttys, sizeof nttys, 1, fp);
	fread (&nchans, sizeof nchans, 1, fp);
	fread (nl, sizeof (struct nlist), nllen, fp);
	fread (allttys, sizeof (struct ttys), nttys, fp);
	if (!nflg)
		if (!(wchanhd = (WCHAN *)calloc(nchans, sizeof(WCHAN)))) {
			fputs("Too many symbols\n",stderr);
			exit(1);
		}
		else for (i = 0, ccc = wchanhd; i < nchans; i++) {
			fread ((char *) ccc, sizeof(WCHAN), 1, fp);
			ccc++;
		}
	return (unamebuf);
}

openfiles (argc, argv)
char	**argv;
{
	kmemf	= "/dev/kmem";

	if (kflg)
		kmemf	= argc > 1 ?  argv[1] : "/usr/sys/core";
	kmem = open (kmemf, 0);
	if (kmem < 0)
		perrexit (kmemf);
	if (!kflg)	{
		memf	= "/dev/mem";
		ismem++;
		}
	else	memf	= kmemf;
	mem	= open (memf, 0);
	if (mem < 0)
		perrexit (memf);
	swapf	= argc > 2 ?  argv[2] : "/dev/swap";
	swap = open (swapf, 0);
	if (swap < 0)
		perrexit (swapf);
}

getkvars(argc,argv)
int	argc;
char	**argv;
{
	nlistf = argc > 3 ?  argv[3] : "/unix";
	if (Uflg) {
		nlist();
		getdev();
		writepsdb();
		exit(0);
	}
	else if (exists(psdb)) {
		uname = readpsdb();
		if (!equal(uname,nlistf)) {
			/*
			 * Let addchan() do the work.
			 */
			nchans = 0;
			free((char *)wchanhd);
			nlist();
		}
	}
	else {
		nlist();
		getdev();
	}

	/* find base of swap */
	lseek(kmem,(off_t)nl[X_SWPLO].n_value,0);
	if (read(kmem,(char *) &swplo,sizeof(swplo)) != sizeof(swplo))
		cantread("swplo",kmemf);

	/* find number of procs */
	if (nl[X_NPROC].n_value) {
		lseek(kmem,(off_t)nl[X_NPROC].n_value,0);
		if (read(kmem,(char *)&nproc,sizeof(nproc)) != sizeof(nproc)) {
			perror(kmemf);
			exit(1);
		}
	}
	else {
		fputs("nproc not in namelist\n",stderr);
		exit(1);
	}
	outargs = (struct psout *)calloc(nproc, sizeof(struct psout));
	if (!outargs) {
		fputs("ps: not enough memory for saving info\n",stderr);
		exit(1);
	}

	/* find value of hz */
	lseek(kmem,(off_t)nl[X_HZ].n_value,0);
	read(kmem,(char *)&hz,sizeof(hz));

	/* locate proc table */
	lseek(kmem,(off_t)nl[X_PROC].n_value,0);
	tell = (off_t)nl[X_PROC].n_value;
}


char *uhdr = "USER       PID NICE SZ TTY       TIME";
upr(a)
register struct psout	*a;
{
	printf("%-8.8s%6u%4d%4d %-8.8s",a->o_uname,a->o_pid,a->o_nice,a->o_size,a->o_tty);
	ptime(a);
}

char *shdr = "   PID TTY       TIME";
spr (a)
register struct psout	*a;
{
	printf("%6u %-8.8s",a->o_pid,a->o_tty);
	ptime(a);
}

char *lhdr = "  F S   UID   PID  PPID CPU PRI NICE  ADDR  SZ WCHAN  TTY       TIME";
lpr(a)
register struct psout *a;
{
	static char	clist[] = "0SWRIZT";

	printf("%3o %c%6u%6u%6u%4d%4d%4d%7o%4d",
0377 & a->o_flag,clist[a->o_stat],a->o_uid,a->o_pid,
a->o_ppid,a->o_cpu & 0377,a->o_pri,a->o_nice,a->o_addr0,a->o_size);
	if (nflg)
		if (a->o_wchan)
			printf("%7o",a->o_wchan);
		else
			fputs("       ",stdout);
	else printf(" %-6.6s",getchan(a->o_wchan));
	printf(" %-8.8s",a->o_tty);
	ptime(a);
}

ptime(a)
struct psout	*a;
{
	time_t	tm;

	tm = (a->o_utime + a->o_stime + 30) / hz;
	printf("%3ld:",tm / 60);
	tm %= 60;
	printf(tm < 10 ? "0%ld" : "%ld",tm);
}

getcmd(a,addr)
off_t	addr;
register struct psout	*a;
{
	/* amount of top of stack to examine for args */
#define ARGLIST	(1024/sizeof(int))
	register	*ip;
	register	char	*cp, *cp1;
	char	c,
		**ap,
		*strcpy();
	int	cc, nbad, abuf [ARGLIST];

	a->o_args[0]	= 0;	/* in case of early return */
	addr	+= ctob ((off_t) mproc->p_ssize) - ARGLIST*sizeof(int);

	/* look for sh special */
	lseek (file, addr + ARGLIST*sizeof(int) - sizeof (char **), 0);
	if (read (file, (char *) &ap, sizeof (char *)) != sizeof (char *))
		return (1);
	if (ap)	{
		char	b[82];
		char	*bp	= b;
		while ((cp = getptr (ap++)) && cp && (bp < b+sizeof (a->o_args)) )	{
			nbad	= 0;
			while ((c = getbyte (cp++)) && (bp < b+sizeof (a->o_args)))	{
				if (c<' ' || c > '~')	{
					if (nbad++ > 3)
						break;
					continue;
					}
				*bp++	= c;
				}
			*bp++	= ' ';
			}
		*bp++	= 0;
		strcpy (a->o_args, b);
		return (1);
		}

	if (pread (file, (char *) abuf, sizeof (abuf), addr) != sizeof (abuf))
		return (1);
	abuf[ARGLIST-1]	= 0;
	for (ip = &abuf[ARGLIST-2]; ip > abuf;)	{
		if (*--ip == -1 || *ip == 0)	{
			cp	= (char *) (ip + 1);
			if (*cp == '\0')
				cp++;
			nbad	= 0;
			for (cp1 = cp; cp1 < (char *) &abuf[ARGLIST]; cp1++)	{
				cc	= *cp1 & 0177;
				if (cc == 0)
					*cp1	= ' ';
				else if (cc < ' ' || cc > 0176)	{
					if (++nbad >= 5)	{
						*cp1++	= ' ';
						break;
						}
					*cp1	= '?';
				} else if (cc == '=')	{
					*cp1	= '\0';
					while (cp1 > cp && *--cp1 != ' ')
						*cp1	= '\0';
					break;
					}
				}
			while (*--cp1 == ' ')
				*cp1	= 0;
			strcpy (a->o_args, cp);
garbage:
			cp	= a->o_args;
			if (cp[0] == '-' && cp[1] <= ' ' || cp[0] == '?' || cp[0] <= ' ')	{
				strcat (cp, " (");
				strcat (cp, u.u_comm);
				strcat (cp, ")");
				}
			cp[63]	= 0;	/* max room in psout is 64 chars */
			if (xflg || gflg || tptr || cp[0] != '-')
				return (1);
			return (0);
			}
		}
	goto garbage;
}

printhdr()
{
	register char	*hdr,
			*cmdstr	= " COMMAND";

	if (rflg)
		return;
	if (lflg && uflg) {
		fputs("ps: specify only one of l and u.\n",stderr);
		exit(1);
	}
	hdr = lflg ? lhdr : (uflg ? uhdr : shdr);
	fputs(hdr,stdout);
	cmdstart = strlen(hdr);
#ifdef TERMCAP
	if (cmdstart + strlen(cmdstr) >= twidth)
		puts(" CMD");
#else !TERMCAP
	puts(cmdstr);
#endif TERMCAP
	fflush(stdout);
}

cantread(what, fromwhat)
	char *what, *fromwhat;
{

	fprintf(stderr, "ps: error reading %s from %s\n", what, fromwhat);
}
