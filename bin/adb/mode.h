#include "machine.h"
/*
 *	UNIX debugger
 */

#define MAXCOM	64
#define MAXARG	32
#define LINSIZ	512
TYPE	int		INT;
TYPE	int		VOID;
TYPE	long int	L_INT;
TYPE	float		REAL;
TYPE	double		L_REAL;
TYPE	unsigned	POS;
TYPE	char		BOOL;
TYPE	char		CHAR;
TYPE	char		*STRING;
TYPE	char		MSG[];
TYPE	struct map	MAP;
TYPE	MAP		*MAPPTR;
TYPE	struct symtab	SYMTAB;
TYPE	SYMTAB		*SYMPTR;
TYPE	struct symslave SYMSLAVE;
TYPE	struct bkpt	BKPT;
TYPE	BKPT		*BKPTR;
TYPE	struct user	U;


/* file address maps */
struct map {
	L_INT	b1;
	L_INT	e1;
	L_INT	f1;
	L_INT	bo;
	L_INT	eo;
	L_INT	fo;
	L_INT	b2;
	L_INT	e2;
	L_INT	f2;
	INT	ufd;
};


/* slave table for symbols */
struct symslave {
	SYMV	valslave;
	SYMFLG	typslave;
	OVTAG	ovlslave;
};

struct bkpt {
	INT	loc;
	INT	ins;
	INT	count;
	INT	initcnt;
	char	flag;
	OVTAG	ovly;
	CHAR	comm[MAXCOM];
	BKPT	*nxtbkpt;
};

TYPE	struct reglist	REGLIST;
TYPE	REGLIST		*REGPTR;
struct reglist {
	STRING	rname;
	INT	roffs;
};

#ifndef NONFP
struct Sfp {
	INT	fpsr;
	REAL	Sfr[6];
};

struct Lfp {
	INT	fpsr;
	L_REAL	Lfr[6];
};
#endif
