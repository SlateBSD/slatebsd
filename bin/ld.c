/*
 * link editor
 */

#include <sys/param.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <a.out.h>
#include <ar.h>

/*
 *	Layout of standard part of a.out file header:
 *		u_int	a_magic;	magic number
 *		u_int	a_text;		text size	)
 *		u_int	a_data;		data size	) in bytes but even
 *		u_int	a_bss;		bss size	)
 *		u_int	a_syms;		symbol table size
 *		u_int	a_entry;	entry point
 *		u_int	a_unused;	(unused)
 *		u_int	a_flag		bit 0 set if no relocation
 *
 *	Layout of overlaid part of a.out file header:
 *		int	max_ovl;	maximum overlay size
 *		u_int	ov_siz[NOVL];	overlay sizes
 *
 *	Non-overlaid offsets:
 *		header:		0
 *		text:		16
 *		data:		16 + a_text
 *		relocation:	16 + a_text + a_data
 *
 *		If relocation info stripped:
 *			symbol table: 16 + a_text + a_data
 *		else
 *			symbol table: 16 + 2 * (a_text + a_data)
 *
 *	Overlaid offsets:
 *		header:		0
 *		overlay header:	16
 *		text:		16 + 2 + 2*NOVL = 16 + 2 + 2*15 = 48
 *		data:		48 + a_text + SUM(ov_siz)
 *		relocation:	48 + a_text + SUM(ov_siz) + a_data
 *
 *		If relocation info stripped:
 *			symbol table: 48 + a_text + SUM(ov_siz) + a_data
 *		else
 *			symbol table: 48 + 2 * (a_text + SUM(ov_siz) + a_data)
 *
 *		where SUM(ov_siz) is the sum of the overlays.
 */

#define TRUE	1
#define FALSE	0

/* #define NSYM	1103		/* 1103 originally */
#define NSYM	2000
#define NROUT	256
/* #define NSYMPR	1000	/* 1000 originally */
#define NSYMPR	800

#define N_COMM	05	/* internal use only; other values in a.out.h */

#define RABS	00
#define RTEXT	02
#define RDATA	04
#define RBSS	06
#define REXT	010

#define RELFLG	01

#define THUNKSIZ	8

/*
 * one entry for each archive member referenced;
 * set in first pass; needs restoring for overlays
 */
typedef struct {
	long	loc;
} LIBLIST;

LIBLIST	liblist[NROUT];
LIBLIST	*libp = liblist;

#define TABSZ	700		/* table of contents stuff */
int tnum;			/* number of symbols in table of contents */
struct tab {
	char	cname[8];
	long	cloc;
} tab[TABSZ];

int	vindex;			/* overlay management */
struct overlay {
	int	argsav;
	int	symsav;
	LIBLIST	*libsav;
	char	*vname;
	u_int	ctsav, cdsav, cbsav;
	u_int	offt, offd, offb;
	long offs;
} vnodes[NOVL + 1];

typedef struct {		/* input management */
	int	nuser;
	int	bno;
	int	nibuf;
	int	buff[256];
} PAGE;
PAGE	page[2];

struct {
	int	nuser;
	int	bno;
} fpage;

typedef struct {
	int	*ptr;
	int	bno;
	int	nibuf;
	int	size;
	PAGE	*pno;
} STREAM;

STREAM	text;
STREAM	reloc;

/*
 * Header from the a.out and the archive it is from (if any).
 */
struct	exec filhdr;
struct	ar_hdr archdr;

/* symbol management */
typedef struct {
	char	n_name[8];
	char	n_type;
	char	sovly;
	u_int	n_value;
	u_int	sovalue;
} SYMBOL;
SYMBOL	symtab[NSYM];		/* actual symbols */
SYMBOL	**symhash[NSYM];	/* ptr to hash table entry */
SYMBOL	*lastsym;		/* last symbol entered */
SYMBOL	*hshtab[NSYM+2];	/* hash table for symbols */
SYMBOL	*p_etext;		/* internal symbols */
SYMBOL	*p_edata;
SYMBOL	*p_end;
SYMBOL	*entrypt;

struct xsymbol {
	char	n_name[8];
	char	n_type;
	char	sovly;
	u_int	n_value;
};
struct xsymbol	cursym;		/* current symbol */
int	nsym;			/* pass2: number of local symbols in a.out */

struct local {
	int locindex;		/* index to symbol in file */
	SYMBOL *locsymbol;	/* ptr to symbol table */
};
struct local	local[NSYMPR];
int	symindex;		/* next available symbol table entry */

/*
 * Options.
 */
int	trace;
int	xflag;		/* discard local symbols */
int	Xflag;		/* discard locals starting with 'L' */
int	Sflag;		/* discard all except locals and globals*/
int	rflag;		/* preserve relocation bits, don't define common */
int	arflag;		/* original copy of rflag */
int	sflag;		/* discard all symbols */
int	Mflag;		/* print rudimentary load map */
int	nflag;		/* pure procedure */
int	Oflag;		/* set magic # to 0405 (overlay) */
int	dflag;		/* define common even with rflag */
int	iflag;		/* I/D space separated */
int	vflag;		/* overlays used */

/*
 * These are the cumulative sizes, set in pass1, which
 * appear in the a.out header when the loader is finished.
 */
u_int	tsize, dsize, bsize;
long	ssize;

/*
 * Symbol relocation:
 */
u_int	ctrel, cdrel, cbrel;

/*
 * The base addresses for the loaded text, data and bass from the
 * current module during pass2 are given by torigin, dorigin and borigin.
 */
u_int	torigin, dorigin, borigin;

/*
 * Errlev is nonzero when errors have occured.
 * Delarg is an implicit argument to the routine delexit
 * which is called on error.  We do ``delarg = errlev'' before normal
 * exits, and only if delarg is 0 (i.e. errlev was 0) do we make the
 * result file executable.
 */
int	errlev;
int	delarg	= 4;

int	ofilfnd;		/* -o given; otherwise move l.out to a.out */
char	*ofilename = "l.out";
int	infil;			/* current input file descriptor */
char	*filname;		/* and its name */
char	tfname[] = "/tmp/ldaXXXXX";

typedef struct {		/* output management */
	int	fildes;
	int	nleft;
	int	*xnext;
	int	iobuf[256];
} BUF;

BUF	toutb;
BUF	doutb;
BUF	troutb;
BUF	droutb;
BUF	soutb;
BUF	voutb;			/* Overlay text goes here */

u_int	torgwas;		/* Saves torigin while doing overlays */
u_int	tsizwas;		/* Saves tsize while doing overlays */
int	numov;			/* Total number of overlays */
int	curov;			/* Overlay being worked on just now */
int	inov;			/* 1 if working on an overlay */
int	ovsize[NOVL+1];		/* The sizes of the overlays */

				/* Kernel overlays have a special
				   subroutine to do the switch */
struct	xsymbol ovhndlr =
	{ "ovhndlr1", N_EXT+N_UNDF, 0, 0 };
#define HNDLR_NUM 7		/* position of ov number in ovhndlr.n_name[] */
u_int	ovbase;			/* The base address of the overlays */

#define	NDIRS	25
#define NDEFDIRS 3		/* number of default directories in dirs[] */
char	*dirs[NDIRS];		/* directories for library search */
int	ndir;			/* number of directories */

SYMBOL	**lookup();
SYMBOL	**slookup();
SYMBOL	*lookloc();

u_int	add();
long	ladd();
int	delexit();
long	lseek();
char	*savestr();
char	*malloc();
char	*mktemp();

main(argc, argv)
char **argv;
{
	register int c, i;
	int num;
	register char *ap, **p;
	char save;
	int found;
	int vscan;

	if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
		signal(SIGINT, delexit);
		signal(SIGTERM, delexit);
	}
	if (argc == 1)
		exit(4);

	/* 
	 * Pull out search directories.
	 */
	for (c = 1; c < argc; c++) {
		ap = argv[c];
		if (ap[0] == '-' && ap[1] == 'L') {
			if (ap[2] == 0)
				error(1, "-L: pathname missing");
			if (ndir >= NDIRS - NDEFDIRS)
				error(1, "-L: too many directories");
			dirs[ndir++] = &ap[2];
		}
	}
	/* add default search directories */
	dirs[ndir++] = "/lib";
	dirs[ndir++] = "/usr/lib";
	dirs[ndir++] = "/usr/local/lib";

	p = argv+1;
	/*
	 * Scan files once to find where symbols are defined.
	 */
	for (c=1; c<argc; c++) {
		if (trace)
			printf("%s:\n", *p);
		filname = 0;
		ap = *p++;
		if (*ap != '-') {
			load1arg(ap);
			continue;
		}
		for (i=1; ap[i]; i++) switch (ap[i]) {

		case 'o':
			if (++c >= argc)
				error(1, "-o where?");
			ofilename = *p++;
			ofilfnd++;
			continue;
		case 'u':
		case 'e':
			if (++c >= argc)
				error(1, "-u or -c: arg missing");
			enter(slookup(*p++));
			if (ap[i]=='e')
				entrypt = lastsym;
			continue;
		case 'D':
			if (++c >= argc)
				error(1, "-D: arg missing");
			num = atoi(*p++);
			if (dsize>num)
				error(1, "-D: too small");
			dsize = num;
			continue;
		case 'l':
			save = ap[--i];
			ap[i]='-';
			load1arg(&ap[i]);
			ap[i]=save;
			goto next;
		case 'M':
			Mflag++;
			continue;
		case 'x':
			xflag++;
			continue;
		case 'X':
			Xflag++;
			continue;
		case 'S':
			Sflag++;
			continue;
		case 'r':
			rflag++;
			arflag++;
			continue;
		case 's':
			sflag++;
			xflag++;
			continue;
		case 'n':
			nflag++;
			continue;
		case 'd':
			dflag++;
			continue;
		case 'i':
			iflag++;
			continue;
		case 't':
			trace++;
			continue;
		case 'L':
			goto next;
		case 'v':
			if (++c >= argc)
				error(1, "-v: arg missing");
			vflag=TRUE;
			vscan = vindex;
			found=FALSE;
			while (--vscan>=0 && found==FALSE)
				found = eq(vnodes[vscan].vname, *p);
			if (found) {
				endload(c, argv);
				restore(vscan);
			} else
				record(c, *p);
			p++;
			continue;
		case 'O':
			Oflag++;
			continue;
		case 'Y':
			if (inov == 0)
				error(1, "-Y: Not in overlay");
			ovsize[curov] = tsize;
			if (trace)
				printf("overlay %d size %d\n", curov, ovsize[curov]);
			curov = inov = 0;
			tsize = tsizwas;
			continue;
		case 'Z':
			if (!inov) {
				tsizwas = tsize;
				if (numov == 0) {
					cursym = ovhndlr;
					enter(lookup());
				}
			}
			else {
				ovsize[curov] = tsize;
				if (trace)
					printf("overlay %d size %d\n", curov, ovsize[curov]);
			}
			tsize = 0;
			inov = 1;
			numov++;
			if (numov > NOVL) {
				printf("ld:too many overlays, max is %d\n",NOVL);
				error(1, (char *)NULL);
			}
			curov++;
			continue;
		case 'y':
		case 'z':
		case 'A':
		case 'H':
		case 'N':
		case 'T':
		default:
			printf("ld:bad flag %c\n",ap[i]);
			error(1, (char *)NULL);
		}
next:
		;
	}
	endload(argc, argv);
	exit(0);
}

delexit()
{
	unlink("l.out");
	if (delarg==0)
		chmod(ofilename, 0777 & ~umask(0));
	exit(delarg);
}

endload(argc, argv)
	int argc;
	char **argv;
{
	register int c, i;
	int dnum;
	register char *ap, **p;

	if (numov)
		rflag = 0;
	filname = 0;
	middle();
	setupout();
	p = argv+1;
	libp = liblist;
	for (c=1; c<argc; c++) {
		ap = *p++;
		if (trace)
			printf("%s:\n", ap);
		if (*ap != '-') {
			load2arg(ap);
			continue;
		}
		for (i=1; ap[i]; i++) switch (ap[i]) {

		case 'D':
			for (dnum = atoi(*p);dorigin < dnum; dorigin += 2) {
				putw(0, &doutb);
				if (rflag)
					putw(0, &droutb);
			}
			/* fall into ... */
		case 'u':
		case 'e':
		case 'o':
		case 'v':
			++c;
			++p;
			/* fall into ... */
		default:
			continue;
		case 'L':
			goto next;
		case 'l':
			ap[--i]='-';
			load2arg(&ap[i]);
			goto next;
		case 'Y':
			roundov();
			inov = 0;
			if (trace)
				printf("end overlay generation\n");
			torigin = torgwas;
			continue;
		case 'Z':
			if (inov == 0)
				torgwas = torigin;
			else
				roundov();
			torigin = ovbase;
			inov = 1;
			curov++;
			continue;
		}
next:
		;
	}
	finishout();
}

/*
 * Scan file to find defined symbols.
 */
load1arg(cp)
	register char *cp;
{
	off_t nloc;
	int kind;

	kind = getfile(cp);
	if (Mflag)
		printf("%s\n", filname);
	switch (kind) {

	/*
	 * Plain file.
	 */
	case 0:
		load1(0, 0L);
		break;

	/*
	 * Archive without table of contents.
	 * (Slowly) process each member.
	 */
	case 1:
		error(-1,
"warning: archive has no table of contents; add one using ranlib(1)");
		nloc = 1;
		while (step(nloc))
			nloc += (archdr.ar_size + sizeof(archdr) + 1) >> 1;
		break;

	/*
	 * Archive with table of contents.
	 * Read the table of contents and its associated string table.
	 * Pass through the library resolving symbols until nothing changes
	 * for an entire pass (i.e. you can get away with backward references
	 * when there is a table of contents!)
	 */
	case 2:
		tnum = archdr.ar_size / sizeof(struct tab);
		if (tnum >= TABSZ)
			error(1, "toc buffer too small");
		lseek(infil, (long)(sizeof(filhdr.a_magic) + sizeof(archdr)), 0);
		read(infil, (char *)tab, tnum * sizeof(struct tab));
		while (ldrand());
		libp->loc = -1;
		libp++;
		break;

	/*
	 * Table of contents is out of date, so search
	 * as a normal library (but skip the __.SYMDEF file).
	 */
	case 3:
		error(-1,
"warning: table of contents for archive is out of date; rerun ranlib(1)");
		for (nloc = 1 + ((archdr.ar_size + sizeof(archdr) + 1) >> 1);
		    step(nloc);
			nloc += (archdr.ar_size + sizeof(archdr) + 1) >> 1);
		break;
	}
	close(infil);
}

/*
 * Advance to the next archive member, which
 * is at offset nloc in the archive.  If the member
 * is useful, record its location in the liblist structure
 * for use in pass2.  Mark the end of the archive in libilst with a -1.
 */
step(nloc)
	off_t nloc;
{
	dseek(&text, nloc, sizeof archdr);
	if (text.size <= 0) {
		libp->loc = -1;
		libp++;
		return (0);
	}
	mget((int *)&archdr, sizeof archdr);
	if (load1(1, nloc + (sizeof archdr) / 2)) {
		libp->loc = nloc;
		libp++;
		if (Mflag)
			printf("\t%s\n", archdr.ar_name);
	}
	return (1);
}

ldrand()
{
	register int i;
	register SYMBOL *sp, **pp;
	LIBLIST *oldp = libp;

	for(i = 0; i < tnum; i++) {
		if ((pp = slookup(tab[i].cname)) == 0)
			continue;
		sp = *pp;
		if (sp->n_type != N_EXT+N_UNDF)
			continue;
		step(tab[i].cloc >> 1);
	}
	return(oldp != libp);
}

/*
 * Examine a single file or archive member on pass 1.
 */
load1(libflg, loc)
	off_t loc;
{
	register SYMBOL *sp;
	int savindex;
	int ndef, type, mtype;
	long nlocal;

	readhdr(loc);
	if (filhdr.a_syms == 0) {
		if (filhdr.a_text+filhdr.a_data == 0)
			return (0);
		error(1, "no namelist");
	}
	ctrel = tsize; cdrel += dsize; cbrel += bsize;
	ndef = 0;
	nlocal = sizeof cursym;
	savindex = symindex;
	if ((filhdr.a_flag&RELFLG)==1) {
		error(1, "No relocation bits");
		return(0);
	}
	loc += (sizeof filhdr)/2 + filhdr.a_text + filhdr.a_data;
	dseek(&text, loc, (int)filhdr.a_syms);
	while (text.size > 0) {
		mget((int *)&cursym, sizeof cursym);
		type = cursym.n_type;
		if (Sflag) {
			mtype = type&037;
			if (mtype==1 || mtype>4) {
				continue;
			}
		}
		if ((type&N_EXT)==0) {
			if (Xflag==0 || cursym.n_name[0]!='L')
				nlocal += sizeof cursym;
			continue;
		}
		symreloc();
		if (enter(lookup()))
			continue;
		if ((sp = lastsym)->n_type != N_EXT+N_UNDF)
			continue;
		if (cursym.n_type == N_EXT+N_UNDF) {
			if (cursym.n_value > sp->n_value)
				sp->n_value = cursym.n_value;
			continue;
		}
		if (sp->n_value != 0 && cursym.n_type == N_EXT+N_TEXT)
			continue;
		ndef++;
		sp->n_type = cursym.n_type;
		sp->n_value = cursym.n_value;
		if ((sp->n_type &~ N_EXT) == N_TEXT)
			sp->sovly = curov;
		if (trace)
			printf("%8.8s in overlay %u at %u\n", sp->n_name, sp->sovly, sp->n_value);
	}
	if (libflg==0 || ndef) {
		tsize = add(tsize,filhdr.a_text,"text overflow");
		dsize = add(dsize,filhdr.a_data,"data overflow");
		bsize = add(bsize,filhdr.a_bss,"bss overflow");
		ssize = ladd(ssize,nlocal,"symbol table overflow");
		return (1);
	}
	/*
	 * No symbols defined by this library member.
	 * Rip out the hash table entries and reset the symbol table.
	 */
	while (symindex>savindex)
		*symhash[--symindex]=0;
	return(0);
}

middle()
{
	register SYMBOL *sp, *symp;
	u_int csize;
	u_int nund, corigin;
	u_int ttsize;

	torigin = 0;
	dorigin = 0;
	borigin = 0;

	p_etext = *slookup("_etext");
	p_edata = *slookup("_edata");
	p_end = *slookup("_end");
	/*
	 * If there are any undefined symbols, save the relocation bits.
	 * (Unless we are overlaying.)
	 */
	symp = &symtab[symindex];
	if (rflag==0 && !numov) {
		for (sp = symtab; sp<symp; sp++)
			if (sp->n_type==N_EXT+N_UNDF && sp->n_value==0
				&& sp!=p_end && sp!=p_edata && sp!=p_etext) {
				rflag++;
				dflag = 0;
				break;
			}
	}
	if (rflag)
		nflag = sflag = iflag = Oflag = 0;
	/*
	 * Assign common locations.
	 */
	csize = 0;
	if (dflag || rflag==0) {
		ldrsym(p_etext, tsize, N_EXT+N_TEXT);
		ldrsym(p_edata, dsize, N_EXT+N_DATA);
		ldrsym(p_end, bsize, N_EXT+N_BSS);
		for (sp = symtab; sp<symp; sp++) {
			register int t;

			if (sp->n_type==N_EXT+N_UNDF && (t = sp->n_value)!=0) {
				t = (t+1) & ~01;
				sp->n_value = csize;
				sp->n_type = N_EXT+N_COMM;
				csize = add(csize, t, "bss overflow");
			}
		}
	}
	if (numov) {
		for (sp = symtab; sp < symp; sp++) {
			if (trace)
				printf("%8.8s n_type %o n_value %o sovalue %o sovly %d\n", sp->n_name, sp->n_type, sp->n_value, sp->sovalue, sp->sovly);
			if (sp->sovly && sp->n_type == N_EXT+N_TEXT) {
				sp->sovalue = sp->n_value;
				sp->n_value = tsize;
				tsize += THUNKSIZ;
				if (trace)
					printf("relocating %s in overlay %d from %o to %o\n",sp->n_name, sp->sovly,sp->sovalue, sp->n_value);
			}
		}
	}
	/*
	 * Now set symbols to their final value
	 */
	if (nflag || iflag)
		tsize = (tsize + 077) & ~077;
	ttsize = tsize;
	if (numov) {
		register int i;

		ovbase = (tsize + 017777) &~ 017777;
		if (trace)
			printf("overlay base is %u.\n", ovbase);
		for (sp = symtab; sp < symp; sp++)
			if (sp->sovly && sp->n_type == N_EXT+N_TEXT) {
				sp->sovalue += ovbase;
				if (trace)
					printf("%.8s at %d overlay %d\n", sp->n_name, sp->sovalue, sp->sovly);
			}
		for (i = 1; i <= NOVL; i++) {
			ovsize[i] = (ovsize[i] + 077) &~ 077;
			if (ovsize[i] > ovsize[0])
				ovsize[0] = ovsize[i];
		}
		if (trace)
			printf("maximum overlay size is %d.\n", ovsize[0]);
		ttsize = ovbase + ovsize[0];
		ttsize = (ttsize + 017777) &~ 017777;
		if (trace)
			printf("overlays end before %u.\n", ttsize);
	}
	dorigin = ttsize;
	if (nflag)
		dorigin = (ttsize+017777) & ~017777;
	if (iflag)
		dorigin = 0;
	corigin = dorigin + dsize;
	borigin = corigin + csize;
	nund = 0;
	for (sp = symtab; sp<symp; sp++)
		switch (sp->n_type) {

		case N_EXT+N_UNDF:
			if (arflag == 0)
				errlev |= 01;
			if ((arflag==0 || dflag) && sp->n_value==0) {
				if (sp==p_end || sp==p_etext || sp==p_edata)
					continue;
				if (nund==0)
					printf("Undefined:\n");
				nund++;
				printf("%.8s\n", sp->n_name);
			}
			continue;
		case N_EXT+N_ABS:
		default:
			continue;
		case N_EXT+N_TEXT:
			sp->n_value += torigin;
			continue;
		case N_EXT+N_DATA:
			sp->n_value += dorigin;
			continue;
		case N_EXT+N_BSS:
			sp->n_value += borigin;
			continue;
		case N_EXT+N_COMM:
			sp->n_type = N_EXT+N_BSS;
			sp->n_value += corigin;
			continue;
		}
	if (sflag || xflag)
		ssize = 0;
	bsize = add(bsize, csize, "bss overflow");
	nsym = ssize / (sizeof cursym);
}

ldrsym(sp, val, type)
	register SYMBOL *sp;
	u_int val;
{
	if (sp == 0)
		return;
	if (sp->n_type != N_EXT+N_UNDF || sp->n_value) {
		printf("%.8s: n_value %o", sp->n_name, sp->n_value);
		error(0, "user attempt to redefine loader-defined symbol");
		return;
	}
	sp->n_type = type;
	sp->n_value = val;
}

setupout()
{
	tcreat(&toutb, 0);
	mktemp(tfname);
	tcreat(&doutb, 1);
	if (sflag==0 || xflag==0)
		tcreat(&soutb, 1);
	if (rflag) {
		tcreat(&troutb, 1);
		tcreat(&droutb, 1);
	}
	if (numov)
		tcreat(&voutb, 1);
	filhdr.a_magic = (Oflag ? A_MAGIC4 : (iflag ? A_MAGIC3 : (nflag ? A_MAGIC2 : A_MAGIC1)));
	if (numov) {
		if (filhdr.a_magic == A_MAGIC1)
			error(1, "-n or -i must be used with overlays");
		filhdr.a_magic |= 020;
	}
	filhdr.a_text = tsize;
	filhdr.a_data = dsize;
	filhdr.a_bss = bsize;
	ssize = sflag? 0 : (ssize + (sizeof cursym) * symindex);
	filhdr.a_syms = ssize&0177777;
	if (entrypt) {
		if (entrypt->n_type!=N_EXT+N_TEXT)
			error(0, "entry point not in text");
		else if (entrypt->sovly)
			error(0, "entry point in overlay");
		else
			filhdr.a_entry = entrypt->n_value | 01;
	} else
		filhdr.a_entry = 0;
	filhdr.a_flag = (rflag==0);
	mput(&toutb, (int *)&filhdr, sizeof filhdr);
	if (numov) {
		register int ovlcnt;

		for (ovlcnt = 0; ovlcnt <= NOVL; ovlcnt++)
			putw(ovsize[ovlcnt], &toutb);
	}
}

load2arg(acp)
char *acp;
{
	register char *cp;
	register LIBLIST *lp;

	cp = acp;
	if (getfile(cp) == 0) {
		while (*cp)
			cp++;
		while (cp >= acp && *--cp != '/');
		mkfsym(++cp);
		load2(0L);
	} else {	/* scan archive members referenced */
		for (lp = libp; lp->loc != -1; lp++) {
			dseek(&text, lp->loc, sizeof archdr);
			mget((int *)&archdr, sizeof archdr);
			mkfsym(archdr.ar_name);
			load2(lp->loc + (sizeof archdr) / 2);
		}
		libp = ++lp;
	}
	close(infil);
}

load2(loc)
long loc;
{
	register SYMBOL *sp;
	register struct local *lp;
	register int symno;
	int type, mtype;

	readhdr(loc);
	ctrel = torigin;
	cdrel += dorigin;
	cbrel += borigin;
	/*
	 * Reread the symbol table, recording the numbering
	 * of symbols for fixing external references.
	 */
	lp = local;
	symno = -1;
	loc += (sizeof filhdr)/2;
	dseek(&text, loc + filhdr.a_text + filhdr.a_data, filhdr.a_syms);
	while (text.size > 0) {
		symno++;
		mget((int *)&cursym, sizeof cursym);
/* inline expansion of symreloc */
		switch (cursym.n_type) {

		case N_TEXT:
		case N_EXT+N_TEXT:
			cursym.n_value += ctrel;
			break;
		case N_DATA:
		case N_EXT+N_DATA:
			cursym.n_value += cdrel;
			break;
		case N_BSS:
		case N_EXT+N_BSS:
			cursym.n_value += cbrel;
			break;
		case N_EXT+N_UNDF:
			break;
		default:
			if (cursym.n_type&N_EXT)
				cursym.n_type = N_EXT+N_ABS;
		}
/* end inline expansion of symreloc */
		type = cursym.n_type;
		if (Sflag) {
			mtype = type&037;
			if (mtype==1 || mtype>4) continue;
		}
		if ((type&N_EXT) == 0) {
			if (!sflag && !xflag &&
			    (!Xflag || cursym.n_name[0] != 'L')) {
				/*
				 * preserve overlay number for locals
				 * mostly for adb.   mjk 7/81
				 */
				if ((type == N_TEXT) && inov)
					cursym.sovly = curov;
				mput(&soutb, (int *)&cursym, sizeof cursym);
			}
			continue;
		}
		if ((sp = *lookup()) == 0)
			error(1, "internal error: symbol not found");
		if (cursym.n_type == N_EXT+N_UNDF ||
		    cursym.n_type == N_EXT+N_TEXT) {
			if (lp >= &local[NSYMPR])
				error(2, "Local symbol overflow");
			lp->locindex = symno;
			lp++->locsymbol = sp;
			continue;
		}
		if (cursym.n_type != sp->n_type
		    || cursym.n_value != sp->n_value && !sp->sovly
		    || sp->sovly && cursym.n_value != sp->sovalue) {
			printf("%.8s: ", cursym.n_name);
			if (trace)
				printf(" sovly %d sovalue %o new %o hav %o ",sp->sovly, sp->sovalue, cursym.n_value, sp->n_value);
			error(0, "multiply defined");
		}
	}
	dseek(&text, loc, filhdr.a_text);
	dseek(&reloc, loc + half(filhdr.a_text + filhdr.a_data), filhdr.a_text);
	load2td(lp, ctrel, inov ? &voutb : &toutb, &troutb);
	dseek(&text, loc+half(filhdr.a_text), filhdr.a_data);
	dseek(&reloc, loc+filhdr.a_text+half(filhdr.a_data), filhdr.a_data);
	load2td(lp, cdrel, &doutb, &droutb);
	torigin += filhdr.a_text;
	dorigin += filhdr.a_data;
	borigin += filhdr.a_bss;
}

load2td(lp, creloc, b1, b2)
	struct local *lp;
	u_int creloc;
	BUF *b1, *b2;
{
	register u_int r, t;
	register SYMBOL *sp;

	for (;;) {
		/*
		 * The pickup code is copied from "get" for speed.
		 */

		/* next text or data word */
		if (--text.size <= 0) {
			if (text.size < 0)
				break;
			text.size++;
			t = get(&text);
		} else if (--text.nibuf < 0) {
			text.nibuf++;
			text.size++;
			t = get(&text);
		} else
			t = *text.ptr++;

		/* next relocation word */
		if (--reloc.size <= 0) {
			if (reloc.size < 0)
				error(1, "relocation error");
			reloc.size++;
			r = get(&reloc);
		} else if (--reloc.nibuf < 0) {
			reloc.nibuf++;
			reloc.size++;
			r = get(&reloc);
		} else
			r = *reloc.ptr++;

		switch (r&016) {

		case RTEXT:
			t += ctrel;
			break;
		case RDATA:
			t += cdrel;
			break;
		case RBSS:
			t += cbrel;
			break;
		case REXT:
			sp = lookloc(lp, r);
			if (sp->n_type==N_EXT+N_UNDF) {
				r = (r&01) + ((nsym+(sp-symtab))<<4) + REXT;
				break;
			}
			t += sp->n_value;
			r = (r&01) + ((sp->n_type-(N_EXT+N_ABS))<<1);
			break;
#ifndef BSD2_10
		default:
			error(1, "relocation format botch (symbol type))");
#endif
		}
		if (r&01)
			t -= creloc;
		putw(t, b1);
		if (rflag)
			putw(r, b2);
	}
}

finishout()
{
	register u_int n, *p;
	register SYMBOL *sp, *symp;

	if (numov) {
		/* int aovno = adrof("__ovno");		XXX KB */
		int aovhndlr[NOVL+1];

		for (n=1; n<=numov; n++) {
			/* Note that NOVL can be up to 15 with this */
			ovhndlr.n_name[HNDLR_NUM] = "0123456789abcdef"[n];
			aovhndlr[n] = adrof(ovhndlr.n_name);
		}
		symp = &symtab[symindex];
		for (sp = symtab; sp < symp; sp++)
			if (sp->sovly && (sp->n_type & (N_EXT+N_TEXT))) {
				putw(012701, &toutb);	/* mov $~foo+4, r1*/
				putw(sp->sovalue+4, &toutb);
				putw(04537, &toutb);	/* jsr r5,ovhndlrx */
				putw(aovhndlr[sp->sovly], &toutb);
				torigin += THUNKSIZ;
			}
	}
	if (nflag||iflag) {
		n = torigin;
		while (n&077) {
			n += 2;
			putw(0, &toutb);
			if (rflag)
				putw(0, &troutb);
		}
	}
	if (numov)
		copy(&voutb);
	copy(&doutb);
	if (rflag) {
		copy(&troutb);
		copy(&droutb);
	}
	if (sflag==0) {
		if (xflag==0)
			copy(&soutb);
		for (p = (u_int *)symtab; p < (u_int *)&symtab[symindex];) {
			/*
			 * this is bad machine dependent code...
			 *
			 * this does the symbol
			 */
			putw(*p++, &toutb); putw(*p++, &toutb);
			putw(*p++, &toutb); putw(*p++, &toutb);

			/* these do the flags and value */
			putw(*p++, &toutb); putw(*p++, &toutb);

			/* skip sovalue */
			p++;
		}
	}
	flush(&toutb);
	close(toutb.fildes);
	if (!ofilfnd) {
		unlink("a.out");
		if (link("l.out", "a.out") < 0)
			error(1, "cannot move l.out to a.out");
		ofilename = "a.out";
	}
	delarg = errlev;
	delexit();
}

mkfsym(s)
char *s;
{

	if (sflag || xflag)
		return;
	cp8c(s, cursym.n_name);
	cursym.n_type = N_TYPE;
	cursym.n_value = torigin;
	mput(&soutb, (int *)&cursym, sizeof cursym);

}

mget(aloc, an)
int *aloc;
{
	register *loc, n;
	register *p;

	n = an;
	n >>= 1;
	loc = aloc;
	if ((text.nibuf -= n) >= 0) {
		if ((text.size -= n) > 0) {
			p = text.ptr;
			do {
				*loc++ = *p++;
			} while (--n);
			text.ptr = p;
			return;
		} else
			text.size += n;
	}
	text.nibuf += n;
	do {
		*loc++ = get(&text);
	} while (--n);
}

dseek(sp, aloc, s)
register STREAM *sp;
long aloc;
int s;
{
	register PAGE *p;
	long b, o;
	int n;

	b = aloc >> 8;
	o = aloc & 0377;
	--sp->pno->nuser;
	if ((p = &page[0])->bno!=b && (p = &page[1])->bno!=b)
		if (p->nuser==0 || (p = &page[0])->nuser==0) {
			if (page[0].nuser==0 && page[1].nuser==0)
				if (page[0].bno < page[1].bno)
					p = &page[0];
			p->bno = b;
			lseek(infil, (aloc & ~0377L) << 1, 0);
			if ((n = read(infil, (char *)p->buff, 512)>>1) < 0)
				n = 0;
			p->nibuf = n;
	} else
		error(1, "botch: no pages");
	++p->nuser;
	sp->bno = b;
	sp->pno = p;
	sp->ptr = p->buff + o;
	if (s != -1)
		sp->size = half(s);
	if ((sp->nibuf = p->nibuf-o) <= 0)
		sp->size = 0;
}

get(sp)
register STREAM *sp;
{
	if (--sp->nibuf < 0) {
		dseek(sp, (long)(sp->bno + 1) << 8, -1);
		--sp->nibuf;
	}
	if (--sp->size <= 0) {
		if (sp->size < 0)
			error(1, "premature EOF");
		++fpage.nuser;
		--sp->pno->nuser;
		sp->pno = (PAGE *)&fpage;
	}
	return(*sp->ptr++);
}

getfile(acp)
char *acp;
{
	struct stat stb;

	archdr.ar_name[0] = '\0';
	filname = acp;
	if (filname[0] == '-' && filname[1] == 'l')
		infil = libopen(filname + 2, O_RDONLY);
	else
		infil = open(filname, O_RDONLY);
	if (infil < 0)
		error(1, "cannot open");
	fstat(infil, &stb);
	page[0].bno = page[1].bno = -1;
	page[0].nuser = page[1].nuser = 0;
	text.pno = reloc.pno = (PAGE *)&fpage;
	fpage.nuser = 2;
	dseek(&text, 0L, 2);
	if (text.size <= 0)
		error(1, "premature EOF");
	if (get(&text) != ARMAG)
		return (0);			/* regular file */
	dseek(&text, 1L, sizeof archdr);	/* word addressing */
	if (text.size <= 0)
		return (1);			/* regular archive */
	mget((int *)&archdr, sizeof archdr);
	if (strncmp(archdr.ar_name, "__.SYMDEF", sizeof(archdr.ar_name)) != 0)
		return (1);			/* regular archive */
	return (stb.st_mtime > archdr.ar_date ? 3 : 2);
}

/*
 * Search for a library with given name
 * using the directory search array.
 */
libopen(name, oflags)
	char *name;
	int oflags;
{
	register char *p, *cp;
	register int i;
	static char buf[MAXPATHLEN+1];
	int fd = -1;

	if (*name == '\0')			/* backwards compat */
		name = "a";
	for (i = 0; i < ndir && fd == -1; i++) {
		p = buf;
		for (cp = dirs[i]; *cp; *p++ = *cp++)
			;
		*p++ = '/';
		for (cp = "lib"; *cp; *p++ = *cp++)
			;
		for (cp = name; *cp; *p++ = *cp++)
			;
		cp = ".a";
		while (*p++ = *cp++)
			;
		fd = open(buf, oflags);
	}
	if (fd != -1)
		filname = buf;
	return (fd);
}

SYMBOL **
lookup()
{
	register SYMBOL **hp;
	register char *cp, *cp1;
	int sh, clash;

	sh = 0;
	for (cp = cursym.n_name; cp < &cursym.n_name[8];)
		sh = (sh<<1) + *cp++;
	for (hp = &hshtab[(sh&077777)%NSYM+2]; *hp!=0;) {
		cp1 = (*hp)->n_name;
		clash=FALSE;
		for (cp = cursym.n_name; cp < &cursym.n_name[8];)
			if (*cp++ != *cp1++) {
				clash=TRUE;
				break;
			}
		if (clash) {
			if (++hp >= &hshtab[NSYM+2])
				hp = hshtab;
		}
		else
			break;
	}
	return(hp);
}

SYMBOL **
slookup(s)
char *s;
{
	cp8c(s, cursym.n_name);
	cursym.n_type = N_EXT+N_UNDF;
	cursym.n_value = 0;
	return(lookup());
}

enter(hp)
SYMBOL **hp;
{
	register SYMBOL *sp;

	if (*hp==0) {
		if (symindex>=NSYM)
			error(1, "symbol table overflow");
		symhash[symindex] = hp;
		*hp = lastsym = sp = &symtab[symindex++];
		cp8c(cursym.n_name, sp->n_name);
		sp->n_type = cursym.n_type;
		sp->n_value = cursym.n_value;
		if (sp->n_type == N_EXT+N_TEXT) {
			sp->sovly = curov;
			if (trace)
				printf("found %8.8s in overlay %d at %u\n",sp->n_name, sp->sovly, sp->n_value);
		}
		return(1);
	} else {
		lastsym = *hp;
		return(0);
	}
}

symreloc()
{
	switch (cursym.n_type) {

	case N_TEXT:
	case N_EXT+N_TEXT:
		cursym.n_value += ctrel;
		return;

	case N_DATA:
	case N_EXT+N_DATA:
		cursym.n_value += cdrel;
		return;

	case N_BSS:
	case N_EXT+N_BSS:
		cursym.n_value += cbrel;
		return;

	case N_EXT+N_UNDF:
		return;

	default:
		if (cursym.n_type&N_EXT)
			cursym.n_type = N_EXT+N_ABS;
		return;
	}
}

error(n, s)
char *s;
{
	if (!s)
		delexit();
	if (errlev==0)
		printf("ld:");
	if (filname) {
		printf("%s", filname);
		if (n != -1 && archdr.ar_name[0])
			printf("(%s)", archdr.ar_name);
		printf(": ");
	}
	printf("%s\n", s);
	if (n == -1)
		return;
	if (n)
		delexit();
	errlev = 2;
}

readhdr(loc)
off_t loc;
{
	dseek(&text, loc, sizeof filhdr);
	mget((int *)&filhdr, sizeof filhdr);
	if (filhdr.a_magic != A_MAGIC1)
		error(1, "bad magic number");
	if (filhdr.a_text&01)
		++filhdr.a_text;
	if (filhdr.a_data&01)
		++filhdr.a_data;
	if (filhdr.a_bss&01)
		++filhdr.a_bss;
	cdrel = -filhdr.a_text;
	cbrel = cdrel - filhdr.a_data;
}

tcreat(buf, tempflg)
BUF *buf;
int tempflg;
{
	register int ufd;
	char *nam;

	nam = (tempflg ? tfname : ofilename);
	if ((ufd = creat(nam, 0666)) < 0)
		error(2, tempflg?"cannot create temp":"cannot create output");
	close(ufd);
	buf->fildes = open(nam, 2);
	if (tempflg)
		unlink(tfname);
	buf->nleft = sizeof(buf->iobuf)/sizeof(int);
	buf->xnext = buf->iobuf;
}

adrof(s)
	char *s;
{
	register SYMBOL **p;

	p = slookup(s);
	if (*p)
		return((*p)->n_value);
	printf("%.8s: ", s);
	error(1, "undefined");
	/*NOTREACHED*/
}

copy(buf)
BUF *buf;
{
	register f, *p, n;

	flush(buf);
	lseek(f = buf->fildes, (long)0, 0);
	while ((n = read(f, (char *)buf->iobuf, sizeof(buf->iobuf))) > 1) {
		n >>= 1;
		p = (int *)buf->iobuf;
		do
			putw(*p++, &toutb);
		while (--n);
	}
	close(f);
}

mput(buf, aloc, an)
BUF *buf;
int *aloc;
{
	register *loc;
	register n;

	loc = aloc;
	n = an>>1;
	do {
		putw(*loc++, buf);
	}
	while (--n);
}

half(i)
int i;
{
	return((i>>1)&077777);
}

SYMBOL *
lookloc(alp, r)
struct local *alp;
{
	register struct local *clp, *lp;
	register int sn;

	lp = alp;
	sn = (r>>4) & 07777;
	for (clp = local; clp<lp; clp++)
		if (clp->locindex == sn)
			return(clp->locsymbol);
	error(1, "local symbol botch");
	/*NOTREACHED*/
}

cp8c(from, to)
register char *from, *to;
{
	register char *te;

	te = to+8;
	while ((*to++ = *from++) && to < te);
	while (to < te)
		*to++ = 0;
}

eq(s1, s2)
register char *s1;
register char *s2;
{
	while (*s1==*s2++)
		if ((*s1++)==0)
			return(TRUE);
	return(FALSE);
}

putw(w, b)
register BUF *b;
{
	*(b->xnext)++ = w;
	if (--b->nleft <= 0)
		flush(b);
}

flush(b)
register BUF *b;
{
	register int n;

	if ((n = (char *)b->xnext - (char *)b->iobuf) > 0)
		if (write(b->fildes, (char *)b->iobuf, n) != n)
			error(1, "output error");
	b->xnext = b->iobuf;
	b->nleft = sizeof(b->iobuf)/sizeof(int);
}

roundov()
{
	while (torigin & 077) {
		putw(0, &voutb);
		torigin += sizeof (int);
	}
}

record(c, nam)
int c;
char *nam;
{
	register struct overlay *v;

	v = &vnodes[vindex++];
	v->argsav = c;
	v->symsav = symindex;
	v->libsav = libp;
	v->vname = nam;
	v->offt = tsize;
	v->offd = dsize;
	v->offb = bsize;
	v->offs = ssize;
	v->ctsav = ctrel;
	v->cdsav = cdrel;
	v->cbsav = cbrel;
}

restore(vscan)
int vscan;
{
	register struct overlay *v;
	register int saved;

	v = &vnodes[vscan];
	vindex = vscan+1;
	libp = v->libsav;
	ctrel = v->ctsav;
	cdrel = v->cdsav;
	cbrel = v->cbsav;
	tsize = v->offt;
	dsize = v->offd;
	bsize = v->offb;
	ssize = v->offs;
	saved = v->symsav;
	while (symindex>saved)
		*symhash[--symindex]=0;
}

long
ladd(a,b,s)
long a, b;
char *s;
{
	long r;

	r = a + b;
	if (r >= 04000000L)
		error(1,s);
	return(r);
}

u_int
add(a,b,s)
int a, b;
char *s;
{
	long r;

	r = (long)(u_int)a + (u_int)b;
	if (r >= 0200000)
		error(1,s);
	return(r);
}
