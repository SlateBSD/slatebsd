#ifndef	lint
char	*sccsid = "@(#)file.c	2.10";
#endif

/*
 * determine type of file
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/inode.h>
#include <a.out.h>
#include <sys/file.h>

int	i, in, ifile;
char buf[BUFSIZ];
char *troff[] = {	/* new troff intermediate lang */
	"x","T","res","init","font","202","V0","p1",0};

char *fort[] = {
	"function","subroutine","common","dimension","block","integer",
	"real","data","double",0};
char *asc[] = {
	"sys","mov","tst","clr","jmp",0};
char *c[] = {
	"int","char","float","double","struct","extern",0};
char *as[] = {
	"globl","byte","even","text","data","bss","comm",0};
char *pas[] = {
	"program", "{", "(*", 0};
char *stp[] = {
	"procedure", "function", "comment", 0};

main(argc, argv)
char **argv;
{
	FILE *fl;
	register char *p;
	char ap[128];

	if (argc>1 && argv[1][0]=='-' && argv[1][1]=='f') {
		if ((fl = fopen(argv[2], "r")) == NULL) {
			perror(argv[2]);
			exit(2);
		}
		while ((p = fgets(ap, sizeof (ap), fl)) != NULL) {
			int l = strlen(p);
			if (l>0)
				p[l-1] = '\0';
			printf("%s:	", p);
			type(p);
			if (ifile>=0)
				close(ifile);
		}
		exit(1);
	}

	while(argc > 1) {
		printf("%s:	", argv[1]);
		type(argv[1]);
		argc--;
		argv++;
		if (ifile >= 0)
			close(ifile);
	}
}

type(file)
char *file;
{
	int j,nl;
	char ch;
	struct stat mbuf;
	int lastnode = 0;

	ifile = -1;

	if(lstat(file, &mbuf) < 0) {
		printf("cannot stat\n");
		perror(file);
		return;
	}	

	switch (mbuf.st_mode & S_IFMT) {

	case S_IFLNK:
		printf("symbolic link\n");
		return;

	case S_IFCHR:
		printf("character ");
		goto spcl;

	case S_IFDIR:
		printf("directory\n");
		return;

	case S_IFBLK:
		printf("block");

spcl:
		printf(" special (%d/%d)\n", major(mbuf.st_rdev), minor(mbuf.st_rdev));
		return;

	}



	ifile = open(file, O_RDONLY);
	if(ifile < 0) {
		printf("cannot open\n");
		perror(file);
		return;
	}
	in = read(ifile, buf, BUFSIZ);
	if(in == 0) {
		printf("empty\n");
		return;
	}
	switch(*(int *)buf) {

	case A_MAGIC2:
		printf("pure ");
		goto exec;

	case A_MAGIC3:
		printf("separate ");
		goto exec;
	
	case A_MAGIC4:
		printf("replacement text");
		goto exec;

	case A_MAGIC5:
		printf("overlaid pure ");
		goto exec;

	case A_MAGIC6:
		printf("overlaid separate ");
		goto exec;

	case 0413:
		printf("demand paged ");
		goto exec;

	case A_MAGIC1:
exec:
		printf("executable");
		if(((int *)buf)[4] != 0)
			printf(" not stripped");
#ifdef BSD2_10
		if(((int *)buf)[1] == 0 &&
		   ((int *)buf)[2] != 0 &&
		   ((int *)buf)[3] == 0)
			printf(" (likely vax)");
#endif BSD2_10
		printf("\n");
		goto out;

	case 0177555:
		printf("old archive\n");
		goto out;

	case 0177545:
		printf("archive\n");
		goto out;
	}
#ifdef vax
	i = *(short *) buf;
	if (i >= 0407 && i <= 0431) {
		printf("pdp-11 binary\n");
		goto out;
	}
#endif

	if (strncmp(buf, "!<arch>\n__.SYMDEF", 17) == 0) {
		printf("new archive random library\n");
		goto out;
	}
	if (strncmp(buf, "!<arch>\n", 8) == 0) {
		printf("new archive\n");
		goto out;
	}
	if (mbuf.st_size % 512 == 0) {	/* it may be a IMAGEN PRESS file */
		lseek(ifile, -512L, L_XTND);	/* last block */
		if (read(file, buf, BUFSIZ) > 0
		  && *(short *)buf == 12138) {
			printf("PRESS file\n");
			goto out;
		}
	}

	if (buf[0] == '\037' && buf[1] == '\235') {
		printf("compressed data, ");
		if (buf[2]&0x80)
			printf("block compressed, ");
		printf("%d bits\n", buf[2]&0x1f);
		goto out;
	}

	i = 0;
	if(ccom() == 0)goto notc;
	while(buf[i] == '#'){
		j = i;
		while(buf[i++] != '\n'){
			if(i - j > 255){
				printf("data\n"); 
				goto out;
			}
			if(i >= in)
				goto notc;
		}
		if(ccom() == 0)
			goto notc;
	}
check:
	if(lookup(c) == 1){
		while((ch = buf[i++]) != ';' && ch != '{')if(i >= in)goto notc;
		printf("c program text");
		goto outa;
	}
	nl = 0;
	while(buf[i] != '('){
		if(buf[i] <= 0)
			goto notas;
		if(buf[i] == ';'){
			i++; 
			goto check; 
		}
		if(buf[i++] == '\n')
			if(nl++ > 6)goto notc;
		if(i >= in)
			goto notc;
	}
	while(buf[i] != ')'){
		if(buf[i++] == '\n')
			if(nl++ > 6)
				goto notc;
		if(i >= in)
			goto notc;
	}
	while(buf[i] != '{'){
		if(buf[i++] == '\n')
			if(nl++ > 6)
				goto notc;
		if(i >= in)
			goto notc;
	}
	printf("c program text");
	goto outa;
notc:
	i = 0;
	while(buf[i] == 'c' || buf[i] == '#'){
		while(buf[i++] != '\n')
			if(i >= in)
				goto notfort;
	}
	if(lookup(fort) == 1){
		printf("fortran program text");
		goto outa;
	}
notfort:
	if(lookup(pas) == 1){
		printf("pascal program text");
		goto outa;
	}
	if(lookup(stp) == 1){
		printf("staple program text");
		goto outa;
	}
	i=0;
	if(ascom() == 0)goto notas;
	j = i-1;
	if(buf[i] == '.'){
		i++;
		if(lookup(as) == 1){
			printf("assembler program text"); 
			goto outa;
		}
		else if(buf[j] == '\n' && isalpha(buf[j+2])){
			printf("roff, nroff, or eqn input text");
			goto outa;
		}
	}
	while(lookup(asc) == 0){
		if(ascom() == 0)
			goto notas;
		while(buf[i] != '\n' && buf[i++] != ':')
			if(i >= in)
				goto notas;
		while(buf[i] == '\n' || buf[i] == ' ' || buf[i] == '\t')
			if(i++ >= in)
				goto notas;
		j = i-1;
		if(buf[i] == '.'){
			i++;
			if(lookup(as) == 1){
				printf("assembler program text"); 
				goto outa; 
			}
			else if(buf[j] == '\n' && isalpha(buf[j+2])){
				printf("roff, nroff, or eqn input text");
				goto outa;
			}
		}
	}
	printf("assembler program text");
	goto outa;
notas:
	for(i=0; i < in; i++)if(buf[i]&0200){
		if (buf[0]=='\100' && buf[1]=='\357') {
			printf("troff output\n");
			goto out;
		}
		printf("data\n"); 
		goto out; 
	}
	if (mbuf.st_mode&((S_IEXEC)|(S_IEXEC>>3)|(S_IEXEC>>6)))
		printf("commands text");
	else
		if (troffint(buf, in))
			printf("troff intermediate output text");
		else
			if (english(buf, in))
				printf("English text");
			else
				printf("ascii text");
outa:
	while(i < in)
		if((buf[i++]&0377) > 127){
			printf(" with garbage\n");
			goto out;
		}
	/* if next few lines in then read whole file looking for nulls ...
		while((in = read(ifile,buf,512)) > 0)
			for(i = 0; i < in; i++)
				if((buf[i]&0377) > 127){
					printf(" with garbage\n");
					goto out;
				}
		/*.... */
	printf("\n");
out:;
}

troffint(bp, n)
char *bp;
int n;
{
	int k;

	i = 0;
	for (k = 0; k < 6; k++) {
		if (lookup(troff) == 0)
			return(0);
		if (lookup(troff) == 0)
			return(0);
		while (i < n && buf[i] != '\n')
			i++;
		if (i++ >= n)
			return(0);
	}
	return(1);
}

lookup(tab)
char *tab[];
{
	char r;
	int k,j,l;
	while(buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n')
		i++;
	for(j=0; tab[j] != 0; j++){
		l=0;
		for(k=i; ((r=tab[j][l++]) == buf[k] && r != '\0');k++)
			;
		if(r == '\0')
			if (!isalnum(buf[k])) {
				i=k;
				return(1);
			}
	}
	return(0);
}
ccom(){
	char cc;
	while((cc = buf[i]) == ' ' || cc == '\t' || cc == '\n')
		if(i++ >= in)
			return(0);
	if(buf[i] == '/' && buf[i+1] == '*'){
		i += 2;
		while(buf[i] != '*' || buf[i+1] != '/'){
			if(buf[i] == '\\')i += 2;
			else
				i++;
			if(i >= in)
				return(0);
		}
		if((i += 2) >= in)
			return(0);
	}
	if(buf[i] == '\n')
		if(ccom() == 0)
			return(0);
	return(1);
}
ascom(){
	while(buf[i] == '/'){
		i++;
		while(buf[i++] != '\n')
			if(i >= in)
				return(0);
		while(buf[i] == '\n')
			if(i++ >= in)
				return(0);
	}
	return(1);
}

english (bp, n)
char *bp;
{
# define NASC 128
	int ct[NASC], j, vow, freq, rare;
	int badpun = 0, punct = 0;
	if (n<50)
		return(0); /* no point in statistics on squibs */
	for(j=0; j<NASC; j++)
		ct[j]=0;
	for(j=0; j<n; j++) {
		if (bp[j]<NASC)
			ct[bp[j]|040]++;
		switch (bp[j]) {
		case '.': 
		case ',': 
		case ')': 
		case '%':
		case ';': 
		case ':': 
		case '?':
			punct++;
			if ( j < n-1 && bp[j+1] != ' ' && bp[j+1] != '\n')
				badpun++;
		}
	}
	if (badpun*5 > punct)
		return(0);
	vow = ct['a'] + ct['e'] + ct['i'] + ct['o'] + ct['u'];
	freq = ct['e'] + ct['t'] + ct['a'] + ct['i'] + ct['o'] + ct['n'];
	rare = ct['v'] + ct['j'] + ct['k'] + ct['q'] + ct['x'] + ct['z'];
	if (2*ct[';'] > ct['e'])
		return(0);
	if ( (ct['>']+ct['<']+ct['/'])>ct['e'])
		return(0); /* shell file test */
	return (vow*5 >= n-ct[' '] && freq >= 10*rare);
}
