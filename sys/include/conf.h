/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)conf.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Declaration of block device
 * switch. Each entry (row) is
 * the only link between the
 * main unix code and the driver.
 * The initialization of the
 * device switches is in the
 * file conf.c.
 */
struct bdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	(*d_root)();		/* XXX root attach routine */
	int	d_flags;
};

#ifdef KERNEL
extern struct	bdevsw bdevsw[];
#endif

/*
 * Character device switch.
 */
struct cdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_ioctl)();
	int	(*d_stop)();
	struct tty *d_ttys;
	int	(*d_select)();
};
#ifdef KERNEL
extern struct	cdevsw cdevsw[];
#endif

/*
 * tty line control switch.
 */
struct linesw
{
	int	(*l_open)();
	int	(*l_close)();
	int	(*l_read)();
	int	(*l_write)();
	int	(*l_ioctl)();
	int	(*l_rint)();
	int	(*l_rend)();
	int	(*l_meta)();
	int	(*l_start)();
	int	(*l_modem)();
};
#ifdef KERNEL
extern struct	linesw linesw[];
#endif
