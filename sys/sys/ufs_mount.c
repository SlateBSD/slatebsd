/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ufs_mount.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "systm.h"
#include "user.h"
#include "inode.h"
#include "fs.h"
#include "buf.h"
#include "mount.h"
#include "file.h"
#include "namei.h"
#include "conf.h"

smount()
{
	register struct a {
		char	*fspec;
		char	*freg;
		int	ronly;
	} *uap = (struct a *)u.u_ap;
	dev_t dev;
	register struct inode *ip;
	register struct fs *fs;
	u_int len;

	u.u_error = getmdev(&dev, uap->fspec);
	if (u.u_error)
		return;
	u.u_segflg = UIO_USERSPACE;
	u.u_dirp = (caddr_t)uap->freg;
	ip = namei(LOOKUP | FOLLOW);
	if (ip == NULL)
		return;
	if (ip->i_count != 1) {
		iput(ip);
		u.u_error = EBUSY;
		return;
	}
	if ((ip->i_mode&IFMT) != IFDIR) {
		iput(ip);
		u.u_error = ENOTDIR;
		return;
	}
	fs = mountfs(dev, uap->ronly, ip);
	if (fs == 0)
		return;
	(void) copyinstr(uap->freg, fs->fs_fsmnt, sizeof(fs->fs_fsmnt)-1, &len);
	bzero(fs->fs_fsmnt + len, sizeof (fs->fs_fsmnt) - len);
}

/* this routine has races if running twice */
struct fs *
mountfs(dev, ronly, ip)
	dev_t dev;
	int ronly;
	struct inode *ip;
{
	register struct mount *mp = 0;
	struct buf *tp = 0;
	register struct fs *fs;
	register error;
	int needclose = 0;

	error =
	    (*bdevsw[major(dev)].d_open)(dev, ronly ? FREAD : FREAD|FWRITE);
	if (error)
		goto out;
	needclose = 1;
	tp = bread(dev, SBLOCK);
	if (tp->b_flags & B_ERROR)
		goto out;
	for (mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if (mp->m_inodp != 0 && dev == mp->m_dev) {
			mp = 0;
			error = EBUSY;
			needclose = 0;
			goto out;
		}
	for (mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if (mp->m_inodp == 0)
			goto found;
	mp = 0;
	error = EMFILE;		/* needs translation */
	goto out;
found:
	mp->m_inodp = ip;	/* reserve slot */
	mp->m_dev = dev;
	fs = &mp->m_filsys;
	bcopy(mapin(tp), (caddr_t)fs, sizeof(struct fs));
	mapout(tp);
	brelse(tp);
	tp = 0;
	fs->fs_ronly = (ronly != 0);
	if (ronly == 0)
		fs->fs_fmod = 1;
	fs->fs_ilock = 0;
	fs->fs_flock = 0;
	fs->fs_nbehind = 0;
	fs->fs_lasti = 1;
	if (ip) {
		ip->i_flag |= IMOUNT;
		IUNLOCK(ip);
	}
	return (fs);
out:
	if (error == 0)
		error = EIO;
	if (ip)
		iput(ip);
	if (mp)
		mp->m_inodp = 0;
	if (tp)
		brelse(tp);
	if (needclose) {
		(*bdevsw[major(dev)].d_close)(dev, ronly? FREAD : FREAD|FWRITE);
		binval(dev);
	}
	u.u_error = error;
	return (0);
}

umount()
{
	struct a {
		char	*fspec;
	} *uap = (struct a *)u.u_ap;

	u.u_error = unmount1(uap->fspec);
}

unmount1(fname)
	caddr_t fname;
{
	dev_t dev;
	register struct mount *mp;
	register struct inode *ip;
	register int error;

	error = getmdev(&dev, fname);
	if (error)
		return (error);
	for (mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if (mp->m_inodp != NULL && dev == mp->m_dev)
			goto found;
	return (EINVAL);
found:
	xumount(dev);	/* remove unused sticky files from text table */
	update();
	if (iflush(dev) < 0)
		return (EBUSY);
	ip = mp->m_inodp;
	ip->i_flag &= ~IMOUNT;
	irele(ip);
	mp->m_inodp = 0;
	mp->m_dev = 0;
	(*bdevsw[major(dev)].d_close)(dev, 0);
	binval(dev);
	return (0);
}

/*
 * Common code for mount and umount.
 * Check that the user's argument is a reasonable
 * thing on which to mount, otherwise return error.
 */
getmdev(pdev, fname)
	caddr_t fname;
	dev_t *pdev;
{
	register dev_t dev;
	register struct inode *ip;

	if (!suser())
		return (u.u_error);
	u.u_segflg = UIO_USERSPACE;
	u.u_dirp = fname;
	ip = namei(LOOKUP | FOLLOW);
	if (ip == NULL) {
		if (u.u_error == ENOENT)
			return (ENODEV); /* needs translation */
		return (u.u_error);
	}
	if ((ip->i_mode&IFMT) != IFBLK) {
		iput(ip);
		return (ENOTBLK);
	}
	dev = (dev_t)ip->i_rdev;
	iput(ip);
	if (major(dev) >= nblkdev)
		return (ENXIO);
	*pdev = dev;
	return (0);
}
