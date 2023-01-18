/*
 *	print symbol tables for
 *	object or archive files
 *
 *	nm [-goprun] [name ...]
*/

#include	<sys/param.h>
#include	<ar.h>
#include	<a.out.h>
#include	<stdio.h>
#include	<ctype.h>

struct	nnlist {	/* symbol table entry */
	char		n_name[8];	/* symbol name */
	char		nn_type;	/* type flag */
	char		nn_ovno;
	unsigned int	n_value;	/* value */
};
#define	SELECT	arch_flg ? arp.ar_name : *argv
int	numsort_flg;
int	undef_flg;
int	revsort_flg = 1;
int	globl_flg;
int	nosort_flg;
int	arch_flg;
int	prep_flg;
struct	ar_hdr	arp;
struct	exec	exp;
FILE	*fi;
long	off;
long	ftell();
char	*malloc();
char	*realloc();

main(argc, argv)
char **argv;
{
	int narg;
	int  compare();

	if (--argc>0 && argv[1][0]=='-' && argv[1][1]!=0) {
		argv++;
		while (*++*argv) switch (**argv) {
		case 'n':		/* sort numerically */
			numsort_flg++;
			continue;

		case 'g':		/* globl symbols only */
			globl_flg++;
			continue;

		case 'u':		/* undefined symbols only */
			undef_flg++;
			continue;

		case 'r':		/* sort in reverse order */
			revsort_flg = -1;
			continue;

		case 'p':		/* don't sort -- symbol table order */
			nosort_flg++;
			continue;

		case 'o':		/* prepend a name to each line */
			prep_flg++;
			continue;

		default:		/* oops */
			fprintf(stderr, "nm: invalid argument -%c\n", *argv[0]);
			exit(1);
		}
		argc--;
	}
	if (argc == 0) {
		argc = 1;
		argv[1] = "a.out";
	}
	narg = argc;
	while(argc--) {
		fi = fopen(*++argv,"r");
		if (fi == NULL) {
			fprintf(stderr, "nm: cannot open %s\n", *argv);
			continue;
		}
		off = sizeof(exp.a_magic);
		fread((char *)&exp, 1, sizeof(exp.a_magic), fi);	/* get magic no. */
		if (exp.a_magic == ARMAG)
			arch_flg++;
		else if (N_BADMAG(exp)) {
			fprintf(stderr, "nm: %s-- bad format\n", *argv);
			continue;
		}
		fseek(fi, 0L, 0);
		if (arch_flg) {
			(void) nextel(fi);
			if (narg > 1)
				printf("\n%s:\n", *argv);
		}
		do {
			long o;
			register i, n, c;
			struct nnlist sym;
			struct nnlist *symp = NULL;
			unsigned ovsizes[1 + NOVL];

			fread((char *)&exp, 1, sizeof(struct exec), fi);
			if (N_BADMAG(exp))		/* archive element not in  */
				continue;	/* proper format - skip it */
			if (exp.a_magic == A_MAGIC5 || exp.a_magic == A_MAGIC6) {
				fread((char *)ovsizes, 1, sizeof ovsizes, fi);
				o	= 0L;
				for (i = 1; i <= NOVL; i++)
					o	+= (long) ovsizes[i];
				fseek(fi, o, 1);
			}
			o = (long)exp.a_text + exp.a_data;
			if ((exp.a_flag & 01) == 0)
				o *= 2;
			fseek(fi, o, 1);
			i = 0;
			while (fread((char *)&sym, sizeof(sym), 1, fi) == 1) {
				if (globl_flg && (sym.nn_type&N_EXT)==0)
					continue;
				switch (sym.nn_type&N_TYPE) {
					case N_UNDF:
						c = 'u';
						if (sym.n_value)
							c = 'c';
						break;

					default:
					case N_ABS:
						c = 'a';
						break;

					case N_TEXT:
						c = 't';
						break;

					case N_DATA:
						c = 'd';
						break;

					case N_BSS:
						c = 'b';
						break;

					case N_FN:
						c = 'f';
						break;

					case N_REG:
						c = 'r';
						break;
				}
				if (undef_flg && c!='u')
					continue;
				if (sym.nn_type&N_EXT)
					c = toupper(c);
				sym.nn_type = c;
				if (symp==NULL)
					symp = (struct nnlist *)malloc(sizeof(struct nlist));
				else {
					symp = (struct nnlist *)realloc(symp, (i+1)*sizeof(struct nlist));
				}
				if (symp == NULL) {
					fprintf(stderr, "nm: out of memory on %s\n", *argv);
					exit(2);
				}
				symp[i++] = sym;
			}
			if (i == 0) {
				fprintf(stderr, "nm: %s-- no name list\n", SELECT);
				continue;
			}
			if (nosort_flg==0)
				qsort(symp, i, sizeof(struct nlist), compare);
			if ((arch_flg || narg>1) && prep_flg==0)
				printf("\n%s:\n", SELECT);
			for (n=0; n<i; n++) {
				if (prep_flg) {
					if (arch_flg)
						printf("%s:", *argv);
					printf("%s:", SELECT);
				}
				c = symp[n].nn_type;
				if (!undef_flg) {
					if (c=='u' || c=='U')
						printf("      ");
					else
						printf(N_FORMAT, symp[n].n_value);
					printf(" %c ", c);
				}
				if (symp[n].nn_ovno)
					printf("%-8.8s %d", symp[n].n_name,
					   symp[n].nn_ovno);
				else
					printf("%.8s", symp[n].n_name);
				printf("\n");
			}
			if (symp)
				free((char *)symp);
		} while(arch_flg && nextel(fi));
		fclose(fi);
	}
	exit(0);
}

compare(p1, p2)
register struct nlist *p1, *p2;
{
	register i;

	if (numsort_flg) {
		if (p1->n_value > p2->n_value)
			return(revsort_flg);
		if (p1->n_value < p2->n_value)
			return(-revsort_flg);
	}
	for(i=0; i<sizeof(p1->n_name); i++)
		if (p1->n_name[i] != p2->n_name[i]) {
			if (p1->n_name[i] > p2->n_name[i])
				return(revsort_flg);
			else
				return(-revsort_flg);
		}
	return(0);
}

nextel(af)
register FILE *af;
{
	register r;

	fseek(af, off, 0);
	r = fread((char *)&arp, 1, sizeof(struct ar_hdr), af);  /* read archive header */
	if (r <= 0)
		return(0);
	if (arp.ar_size & 1)
		++arp.ar_size;
	off = ftell(af) + arp.ar_size;	/* offset to next element */
	return(1);
}
