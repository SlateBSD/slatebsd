/*
 * adb: UNIX debugger
 */

#include "defs.h"

MSG             LONGFIL;
MSG             NOTOPEN;
MSG             A68BAD;
MSG             A68LNK;
MSG             BADMOD;

MAP             txtmap;
MAP             datmap;
OVTAG           curov;
int             overlay;

SYMTAB          symbol;
INT             lastframe;
INT             kernel;
INT             callpc;

INT             infile;
INT             outfile;
CHAR            *lp;
INT             maxoff;
INT             maxpos;
INT             octal;

/* symbol management */
L_INT           localval;

/* breakpoints */
BKPTR           bkpthead;

REGLIST reglist [] = {
		"ps", RPS,
		"pc", PC,
		"sp", R6,
		"r5", R5,
		"r4", R4,
		"r3", R3,
		"r2", R2,
		"r1", R1,
		"r0", R0
};

REGLIST kregs[] = {
		"sp", KSP,
		"r5", KR5,
		"r4", KR4,
		"r3", KR3,
		"r2", KR2,
		"r1", KR1,
		"r0", KR0
};

STRING  ovname  = "ov";   /* not in reglist (not from kernel stack) */
INT             frnames[] = { 0, 3, 4, 5, 1, 2 };

char            lastc;
POS             corhdr[];
POS             *uar0;

INT             fcor;
STRING          errflg;
INT             signo;


L_INT           dot;
L_INT           var[];
STRING          symfil;
STRING          corfil;
INT             pid;
L_INT           adrval;
INT             adrflg;
L_INT           cntval;
INT             cntflg;
int             overlay;


/* general printing routines ($) */

printtrace(modif)
{
	INT             narg, i, stat, name, limit;
	POS             dynam;
	REG BKPTR       bkptr;
	CHAR            hi, lo;
	INT             word;
	INT		stack;
	STRING          comptr;
	L_INT           argp, frame, link;
	SYMPTR          symp;
	OVTAG           savov;

	IF cntflg==0 THEN cntval = -1; FI

	switch (modif) {

	    case '<':
			IF cntval == 0
			THEN	WHILE readchar() != EOR
				DO OD
				lp--;
				break;
			FI
			IF rdc() == '<'
			THEN	stack = 1;
			ELSE	stack = 0; lp--;
			FI
			/* fall thru ... */
	    case '>':
		{
			CHAR		file[64];
			CHAR		Ifile[128];
			extern CHAR	*Ipath;
			INT             index;

			index=0;
			IF rdc()!=EOR
			THEN    REP file[index++]=lastc;
				    IF index>=63 THEN error(LONGFIL); FI
				PER readchar()!=EOR DONE
				file[index]=0;
				IF modif=='<'
				THEN	IF Ipath THEN
						strcpy(Ifile, Ipath);
						strcat(Ifile, "/");
						strcat(Ifile, file);
					FI
					IF strcmp(file, "-") != 0
					THEN	iclose(stack, 0);
						infile=open(file,0);
						IF infile<0
						THEN	infile = open(Ifile, 0);
						FI
					ELSE	lseek(infile, 0L, 0);
					FI
					IF infile<0
					THEN    infile=0; error(NOTOPEN);
					ELSE	IF cntflg
						THEN var[9] = cntval;
						ELSE var[9] = 1;
						FI
					FI
				ELSE    oclose();
					outfile=open(file,1);
					IF outfile<0
					THEN    outfile=creat(file,0644);
					ELSE    lseek(outfile,0L,2);
					FI
				FI

			ELSE	IF modif == '<'
				THEN	iclose(-1, 0);
				ELSE	oclose();
				FI
			FI
			lp--;
		}
		break;

	    case 'o':
		octal = TRUE; break;

	    case 'd':
		octal = FALSE; break;

	    case 'q': case 'Q': case '%':
		done();

	    case 'w': case 'W':
		maxpos=(adrflg?adrval:MAXPOS);
		break;

	    case 's': case 'S':
		maxoff=(adrflg?adrval:MAXOFF);
		break;

	    case 'v': 
		prints("variables\n");
		FOR i=0;i<=35;i++
		DO IF var[i]
		   THEN printc((i<=9 ? '0' : 'a'-10) + i);
			printf(" = %Q\n",var[i]);
		   FI
		OD
		break;

	    case 'm': case 'M':
		printmap("? map",&txtmap);
		printmap("/ map",&datmap);
		break;

	    case 0: case '?':
		IF pid
		THEN printf("pcs id = %d\n",pid);
		ELSE prints("no process\n");
		FI
		sigprint(); flushbuf();

	    case 'r': case 'R':
		printregs();
		return;

	    case 'f': case 'F':
		printfregs(modif=='F');
		return;

	    case 'c': case 'C':
		frame=(adrflg?adrval:(kernel?corhdr[KR5]:uar0[R5]))&EVEN;
		lastframe=0;
		savov = curov;
		callpc=(adrflg?get(frame+2,DSP):(kernel?(-2):uar0[PC]));
		WHILE cntval--
		DO      chkerr();
 			printf("%07O: ", frame); /* Add frame address info */
			narg = findroutine(frame);
			printf("%.8s(", symbol.symc);
			argp = frame+4;
			IF --narg >= 0
			THEN    printf("%o", get(argp, DSP));
			FI
			WHILE --narg >= 0
			DO      argp += 2;
				printf(",%o", get(argp, DSP));
			OD
 			/* Add return-PC info.  Force printout of
 			 * symbol+offset (never just a number! ) by using
 			 * max possible offset.  Overlay has already been set
 			 * properly by findfn.
 			 */
 			prints(") return-pc ");
 			{
				INT savmaxoff = maxoff;

 				maxoff = ((unsigned)-1)>>1;
 				psymoff((L_INT)callpc,ISYM,"");
 				maxoff = savmaxoff;
 			}
 			prints("\n");

			IF modif=='C'
			THEN WHILE localsym(frame)
			     DO word=get(localval,DSP);
				printf("%8t%.8s:%10t", symbol.symc);
				IF errflg THEN prints("?\n"); errflg=0; ELSE printf("%o\n",word); FI
			     OD
			FI

			lastframe=frame;
			frame=get(frame, DSP)&EVEN;
			IF kernel? ((u_int)frame>((u_int)0140000+ctob(USIZE))):
			    (frame==0)
				THEN break; FI
		OD
		if (overlay)
			setovmap(savov);
		break;

	    /*print externals*/
	    case 'e': case 'E':
		symset();
		WHILE (symp=symget())
		DO chkerr();
		   IF (symp->symf)==043 ORF (symp->symf)==044
		   THEN printf("%.8s:%12t%o\n", symp->symc, get(leng(symp->symv),DSP));
		   FI
		OD
		break;

	    case 'a': case 'A':
		frame=(adrflg ? adrval : uar0[R4]);

		WHILE cntval--
		DO chkerr();
		   stat=get(frame,DSP); dynam=get(frame+2,DSP); link=get(frame+4,DSP);
		   IF modif=='A'
		   THEN printf("%8O:%8t%-8o,%-8o,%-8o",frame,stat,dynam,link);
		   FI
		   IF stat==1 THEN break; FI
		   IF errflg THEN error(A68BAD); FI

		   IF get(link-4,ISP)!=04767
		   THEN IF get(link-2,ISP)!=04775
			THEN error(A68LNK);
			ELSE /*compute entry point of routine*/
			     prints(" ? ");
			FI
		   ELSE printf("%8t");
			valpr(name=shorten(link)+get(link-2,ISP),ISYM);
			name=get(leng(name-2),ISP);
			printf("%8t\""); limit=8;
			REP word=get(leng(name),DSP); name += 2;
			    lo=word&LOBYTE; hi=(word>>8)&LOBYTE;
			    printc(lo); printc(hi);
			PER lo ANDF hi ANDF limit-- DONE
			printc('"');
		   FI
		   limit=4; i=6; printf("%24targs:%8t");
		   WHILE limit--
		   DO printf("%8t%o",get(frame+i,DSP)); i += 2; OD
		   printc(EOR);

		   frame=dynam;
		OD
		errflg=0;
		flushbuf();
		break;

	    /*set default c frame*/
	    /*print breakpoints*/
	    case 'b': case 'B':
		printf("breakpoints\ncount%8tbkpt%24tcommand\n");
		savov = curov;
		FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
		DO IF bkptr->flag
		   THEN printf("%-8.8d",bkptr->count);
			curov = bkptr->ovly;
			psymoff(leng(bkptr->loc),ISYM,"%24t");
			comptr=bkptr->comm;
			WHILE *comptr DO printc(*comptr++); OD
		   FI
		OD
		curov = savov;
		break;

	    default: error(BADMOD);
	}

}

printmap(s,amap)
STRING  s; MAP *amap;
{
	int file;
	file=amap->ufd;
	printf("%s%12t`%s'\n",s,(file<0 ? "-" : (file==fcor ? corfil : symfil)));
	printf("b1 = %-16Q",amap->b1);
	printf("e1 = %-16Q",amap->e1);
	printf("f1 = %-16Q",amap->f1);
	IF amap->bo
	THEN    printf("\n\t{ bo = %-16Q",amap->bo);
		printf("eo = %-16Q",amap->eo);
		printf("fo = %-16Q}",amap->fo);
	FI
	printf("\nb2 = %-16Q",amap->b2);
	printf("e2 = %-16Q",amap->e2);
	printf("f2 = %-16Q",amap->f2);
	printc(EOR);
}

printfregs(longpr)
{
#ifndef NONFP
	REG i;
	L_REAL f;
	struct Lfp *pfp;

	pfp = (struct Lfp *)&((U*)corhdr)->u_fps.u_fpsr;
	printf("fpsr\t%o\n", pfp->fpsr);
	FOR i=0; i<FRMAX; i++
	DO      IF ((U*)corhdr)->u_fps.u_fpsr&FD ORF longpr /* long mode */
		THEN    f = pfp->Lfr[frnames[i]];
		ELSE    f = ((struct Sfp *)pfp)->Sfr[frnames[i]];
		FI
		printf("fr%-8d%-32.18f\n", i, f);
	OD
#endif
}

printregs()
{
	REG REGPTR      p;
	INT             v;

	IF kernel
	THEN    FOR p=kregs; p<&kregs[7]; p++
		DO      printf("%s%8t%o%8t", p->rname, v=corhdr[p->roffs]);
			valpr(v,DSYM);
			printc(EOR);
		OD
	ELSE    FOR p=reglist; p < &reglist[NREG]; p++
		DO      printf("%s%8t%o%8t", p->rname, v=uar0[p->roffs]);
			valpr(v,(p->roffs==PC?ISYM:DSYM));
			printc(EOR);
		OD
		IF overlay
		THEN    setovmap(((U *)corhdr)->u_ovdata.uo_curov);
			var[VARC] = curov;
			printf("%s%8t%o\n", ovname, curov);
		FI
		printpc();
	FI
}

getreg(regnam)
{
	REG REGPTR      p;
	REG STRING      regptr;
	CHAR            regnxt;

	IF kernel THEN return(NOREG); FI        /* not supported */
	regnxt=readchar();
	FOR p=reglist; p<&reglist[NREG]; p++
	DO      regptr=p->rname;
		IF (regnam == *regptr++) ANDF (regnxt == *regptr)
		THEN    return(p->roffs);
		FI
	OD
	IF regnam==ovname[0] ANDF regnxt==ovname[1]
	THEN    return((POS *)&(((U *)corhdr)->u_ovdata.uo_curov) - uar0);
	FI
	lp--;
	return(NOREG);
}

printpc()
{
	dot=uar0[PC];
	psymoff(dot,ISYM,":%16t"); printins(0,ISP,chkget(dot,ISP));
	printc(EOR);
}

sigprint()
{
	extern char	*sys_siglist[];		/* signal list */

	if (signo >= 0 && signo < NSIG)
		printf("%s",sys_siglist[signo]);
	else
		printf("unknown signal %d",signo);
}
