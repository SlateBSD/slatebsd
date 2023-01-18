/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mount.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	dev_t	m_dev;		/* device mounted */
	struct	fs m_filsys;	/* superblock data */
	struct	inode *m_inodp;	/* pointer to mounted on inode */
};
#ifdef KERNEL
extern struct	mount mount[NMOUNT];
#endif
