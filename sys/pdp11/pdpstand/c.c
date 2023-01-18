/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)c.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

devread(io)
	register struct iob *io;
{

	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io,READ) );
}

devwrite(io)
	register struct iob *io;
{
	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io, WRITE) );
}

devopen(io)
	register struct iob *io;
{
	(*devsw[io->i_ino.i_dev].dv_open)(io);
}

devclose(io)
	register struct iob *io;
{
	(*devsw[io->i_ino.i_dev].dv_close)(io);
}

nullsys()
{ ; }

int	xpstrategy();
int	rpstrategy();
int	rkstrategy();
int	hkstrategy();
int	rlstrategy();
int	sistrategy();
int	rastrategy(), raopen(), raclose();
int	nullsys();
int	tmstrategy(), tmrew(), tmopen();
int	htstrategy(), htopen(), htclose();
int	tsstrategy(), tsopen(), tsclose();
struct devsw devsw[] = {
	"xp",	xpstrategy,	nullsys,	nullsys,
	"rp",	rpstrategy,	nullsys,	nullsys,
	"rk",	rkstrategy,	nullsys,	nullsys,
	"hk",	hkstrategy,	nullsys,	nullsys,
	"rl",	rlstrategy,	nullsys,	nullsys,
	"si",	sistrategy,	nullsys,	nullsys,
	"ra",	rastrategy,	raopen,		raclose,
	"tm",	tmstrategy,	tmopen,		tmrew,
	"ht",	htstrategy,	htopen,		htclose,
	"ts",	tsstrategy,	tsopen,		tsclose,
	0,0,0,0
};
