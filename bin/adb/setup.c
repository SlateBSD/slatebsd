/*
 *
 *      UNIX debugger
 *
 */

#include "defs.h"


MSG             BADNAM;
MSG             BADFIL;

MAP             txtmap;
MAP             datmap;
SYMSLAVE        *symvec;
INT             wtflag;
INT             kernel;
INT             fcor;
INT             fsym;
L_INT           maxfile;
L_INT           maxstor;
L_INT           txtsiz;
L_INT           datsiz;
L_INT           bss;
L_INT           datbas;
L_INT           stksiz;
STRING          errflg;
INT             magic;
L_INT           symbas;
L_INT           symnum;
L_INT           entrypt;
L_INT           var[];

long tell();
INT             argcount;
INT             signo;
POS             corhdr[ctob(USIZE)/sizeof(POS)];
POS             *uar0 = UAR0;

STRING          symfil  = "a.out";
STRING          corfil  = "core";

OVLVEC          ovlseg;
L_INT           ovlsiz;
L_INT           ovloff[NOVL+1];
OVTAG           startov;
int             overlay;

#define TXTHDRSIZ       (sizeof(TXTHDR))
#define OVLHDRSIZ       (sizeof(ovlseg))

#define HBIT    0100

setsym()
{
	INT             relflg;
	SYMSLAVE        *symptr;
	SYMPTR          symp;
	TXTHDR          txthdr;
	CHAR		*sbrk();

	fsym=getfile(symfil,1);
	txtmap.ufd=fsym;
	IF read(fsym, txthdr, TXTHDRSIZ)==TXTHDRSIZ
	THEN    magic=txthdr[0];
		txtsiz = txthdr[1];
		datsiz = txthdr[2];
		bss = txthdr[3];
		symnum = txthdr[4]/SYMTABSIZ;
		entrypt = txthdr[5];
		relflg = txthdr[7];
		symbas = txtsiz+datsiz;
		switch (magic)
		{
			INT ovly;

			case 0407:
				txtmap.b1 = txtmap.b2 = 0;
				txtmap.e1 = txtmap.e2 = txtsiz+datsiz;
				txtmap.f1 = txtmap.f2 = TXTHDRSIZ;
				break;

			case 0410:
				txtmap.b1 = 0;
				txtmap.e1 = txtsiz;
				txtmap.f1 = TXTHDRSIZ;
				txtmap.b2 = round(txtsiz, TXTRNDSIZ);
				txtmap.e2 = txtmap.b2+datsiz;
				txtmap.f2 = txtsiz+TXTHDRSIZ;
				break;

			case 0405:
			case 0411:
				txtmap.b1 = 0;
				txtmap.e1 = txtsiz;
				txtmap.f1 = TXTHDRSIZ;
				txtmap.b2 = 0;
				txtmap.e2 = datsiz;
				txtmap.f2 = txtsiz+TXTHDRSIZ;
				break;

			case 0430:
			case 0431:
				IF read(fsym, &ovlseg, OVLHDRSIZ) == OVLHDRSIZ
				THEN    txtmap.b1 = 0;
					txtmap.e1 = txtsiz;
					txtmap.f1 = TXTHDRSIZ+OVLHDRSIZ;
					txtmap.bo = round(txtsiz, TXTRNDSIZ);
					txtmap.eo = 0;
					txtmap.fo = 0;
					FOR ovly = 0; ovly < NOVL; ovly++
					DO      ovloff[ovly] = ovlsiz + txtsiz 
							+TXTHDRSIZ+OVLHDRSIZ;
						ovlsiz += ovlseg.ov[ovly];
					OD
					IF magic == 0430
					THEN    txtmap.b2 =
						round(txtmap.bo+(long)ovlseg.max, TXTRNDSIZ);
					ELSE    txtmap.b2 = 0;
					FI
					txtmap.f2 = TXTHDRSIZ+OVLHDRSIZ+txtsiz+ovlsiz;
					symbas += ovlsiz+OVLHDRSIZ;
					txtmap.e2 = txtmap.b2 + datsiz;
					overlay = 1;
					break;
				FI

			default:        magic = 0;
					txtsiz = 0;
					datsiz = 0;
					bss = 0;
					symnum = 0;
					entrypt = 0;
					relflg = 0;
					symbas = 0;
		}
		datbas = txtmap.b2;
		IF relflg!=1 THEN symbas <<= 1; FI
		symbas += TXTHDRSIZ;

		/* set up symvec */
		symvec=(SYMSLAVE *)sbrk(shorten((1+symnum))*sizeof (SYMSLAVE));
		IF (symptr=symvec)==-1
		THEN    printf("%s\n",BADNAM);
			symptr=symvec=(SYMSLAVE *)sbrk(sizeof (SYMSLAVE));
		ELSE if (symnum != 0) {
			symset();
			WHILE (symp=symget()) ANDF errflg==0
			DO  symptr->valslave=symp->symv;
			    symptr->typslave=SYMTYPE(symp->symf);
			    symptr->ovlslave = symp->ovnumb;
			    symptr++;
			OD
		    }
		FI
		symptr->typslave=ESYM;
	FI
	IF magic==0 THEN txtmap.e1=maxfile; FI
}


setcor()
{
	fcor=getfile(corfil,2);
	datmap.ufd=fcor;
	IF read(fcor, corhdr, sizeof corhdr)==sizeof corhdr
	THEN    IF !kernel
		THEN    txtsiz = ((U*)corhdr)->u_tsize << 6;
			datsiz = ((U*)corhdr)->u_dsize << 6;
			stksiz = ((U*)corhdr)->u_ssize << 6;
			datmap.f1 = ctob(USIZE);
			datmap.b2 = maxstor-stksiz;
			datmap.e2 = maxstor;
		ELSE    datsiz = round(datsiz+bss,64L);
			stksiz = (long) ctob(USIZE);
			datmap.f1 = 0;
			datmap.b2 = 0140000L;
			datmap.e2 = 0140000L + ctob(USIZE);
		FI
		switch (magic)
		{
			INT ovly;

			case 0407:
				datmap.b1 = 0;
				datmap.e1 = txtsiz+datsiz;
				IF kernel
				THEN    datmap.f2 = (long)corhdr[KA6] *
						0100L;
				ELSE    datmap.f2 = ctob(USIZE)+txtsiz+datsiz;
				FI
				break;

			case 0410:
				datmap.b1 = round(txtsiz, TXTRNDSIZ);
				datmap.e1 = datmap.b1+datsiz;
				datmap.f2 = datsiz+ctob(USIZE);
				break;

			case 0405:
			case 0411:
			case 0431:
				datmap.b1 = 0;
				datmap.e1 = datsiz;
				IF kernel
				THEN datmap.f2 = (long)corhdr[KA6] *
					0100L;
				ELSE datmap.f2 = datsiz+ctob(USIZE);
				FI
				break;

			case 0430:
				datmap.b1 = round(round(txtsiz,
					TXTRNDSIZ)+ovlseg.max,
					TXTRNDSIZ);
				datmap.e1 = datmap.b1+datsiz;
				IF kernel
				THEN    datmap.b1 = 0;
					datmap.f2 = (long)corhdr[KA6] *
						0100L;
				ELSE    datmap.f2 = datsiz+ctob(USIZE);
				FI
				break;

			default:
				magic = 0;
				datmap.b1 = 0;
				datmap.e1 = maxfile;
				datmap.f1 = 0;
				datmap.b2 = 0;
				datmap.e2 = 0;
				datmap.f2 = 0;
		}
		datbas = datmap.b1;
		if (!kernel && magic) {
			/*
			 * Note that we can no longer compare the magic
			 * number of the core against that of the object
			 * since the user structure no longer contains
			 * the exec header ...
			 */
			register POS *ar0;
			ar0 = (POS *)(((U *)corhdr)->u_ar0);
			if (ar0 > (POS *)0140000
			    && ar0 < (POS *)(0140000 + ctob(USIZE))
			    && !((unsigned)ar0&01))
				uar0 = (POS *)&corhdr[ar0-(POS *)0140000];
			if (overlay) {
				startov = ((U *)corhdr)->u_ovdata.uo_curov;
				var[VARC] = (long)startov;
				setovmap(startov);
			}
			/* else dig out __ovno if overlaid? */
		}
	ELSE    datmap.e1 = maxfile;
	FI
}

create(f)
STRING  f;
{       int fd;
	IF (fd=creat(f,0644))>=0
	THEN close(fd); return(open(f,wtflag));
	ELSE return(-1);
	FI
}

getfile(filnam,cnt)
STRING  filnam;
{
	REG INT         fsym;

	IF !eqstr("-",filnam)
	THEN    fsym=open(filnam,wtflag);
		IF fsym<0 ANDF argcount>cnt
		THEN    IF wtflag
			THEN    fsym=create(filnam);
			FI
			IF fsym<0
			THEN printf("cannot open `%s'\n", filnam);
			FI
		FI
	ELSE    fsym = -1;
	FI
	return(fsym);
}
