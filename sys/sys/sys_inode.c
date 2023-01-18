/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)sys_inode.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/seg.h"

#include "user.h"
#include "proc.h"
#include "inode.h"
#include "buf.h"
#include "fs.h"
#include "ioctl.h"
#include "file.h"
#include "stat.h"
#include "mount.h"
#include "conf.h"
#include "uio.h"
#include "kernel.h"
#include "systm.h"

readi(ip)
	register struct inode *ip;
{
	register struct buf *bp;
	register int n;
	daddr_t lbn, bn;
	off_t diff;
	dev_t dev;
	int on, type;

	if (!u.u_count)
		return;
	if (u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	ip->i_flag |= IACC;
	dev = ip->i_rdev;
	type = ip->i_mode&IFMT;
	if (type == IFCHR) {
		u.u_error = (*cdevsw[major(dev)].d_read)(dev);
		return;
	}
	do {
		lbn = bn = lblkno(u.u_offset);
		on = blkoff(u.u_offset);
		n = MIN((u_int)(DEV_BSIZE - on), u.u_count);
		if (type != IFBLK) {
			diff = ip->i_size - u.u_offset;
			if (diff <= 0)
				return;
			if (diff < n)
				n = diff;
			bn = bmap(ip, bn, B_READ, 0);
			if (u.u_error)
				return;
			dev = ip->i_dev;
		} else
			rablock = bn+1;
		if (bn < 0) {
			bp = geteblk();
			bp->b_resid = 0;
			clrbuf(bp);
		}
		else {
			if (ip->i_lastr + 1 == lbn)
				bp = breada(dev, bn, rablock);
			else
				bp = bread(dev, bn);
			if (bp->b_flags & B_ERROR) {
				u.u_error = EIO;
				brelse(bp);
				return;
			}
		}
		ip->i_lastr = lbn;
		n = MIN((u_int)n, DEV_BSIZE - bp->b_resid);
		if (n != 0) {
			u.u_error = uiomove(mapin(bp) + on, n, UIO_READ);
			mapout(bp);
		}
		if (n + on == DEV_BSIZE || u.u_offset == ip->i_size) {
			if (ip->i_flag & IPIPE)
				bp->b_flags &= ~B_DELWRI;
			bp->b_flags |= B_AGE;
		}
		brelse(bp);
	} while (!u.u_error && u.u_count && n > 0);
}

writei(ip)
	register struct inode *ip;
{
	register struct buf *bp;
	register int on;
	daddr_t bn;
	dev_t dev;
	int n, type;

	if (u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	dev = ip->i_rdev;
	type = ip->i_mode&IFMT;
	/*
	 * technically, next three lines should probably be done *after*
	 * the write has been attempted.  If you move them, make sure you
	 * set the IUPD|ICHG before calling the cdevsw routine.
	 */
	if (u.u_ruid != 0)		/* clear set-uid/gid unless root */
		ip->i_mode &= ~(ISUID|ISGID);
	ip->i_flag |= IUPD|ICHG;
	if (type == IFCHR) {
		u.u_error = (*cdevsw[major(dev)].d_write)(dev);
		return;
	}
	if (!u.u_count)
		return;
	if (type == IFREG && u.u_offset + u.u_count >
	      u.u_rlimit[RLIMIT_FSIZE].rlim_cur) {
		psignal(u.u_procp, SIGXFSZ);
		u.u_error = EFBIG;
		return;
	}
	do {
		bn = lblkno(u.u_offset);
		on = blkoff(u.u_offset);
		n = MIN((u_int)(DEV_BSIZE - on), u.u_count);
		if (type != IFBLK) {
			bn = bmap(ip, bn, B_WRITE, n == DEV_BSIZE ? 0 : 1);
			if (bn < 0)
				return;
			dev = ip->i_dev;
		}
		if (n == DEV_BSIZE)
			bp = getblk(dev, bn);
		else {
			bp = bread(dev, bn);
			if (bp->b_flags & B_ERROR) {
				u.u_error = EIO;
				brelse(bp);
				return;
			}
			/*
			 * Tape drivers don't clear buffers on end-of-tape
			 * any longer (clrbuf can't be called from interrupt).
			 */
			if (bp->b_resid == DEV_BSIZE) {
				bp->b_resid = 0;
				clrbuf(bp);
			}
		}
		u.u_error = uiomove(mapin(bp) + on, n, UIO_WRITE);
		mapout(bp);
		if (u.u_error)
			brelse(bp);
		else if ((ip->i_mode&IFMT) == IFDIR)
			bwrite(bp);
		else if (n + on == DEV_BSIZE && !(ip->i_flag & IPIPE)) {
			bp->b_flags |= B_AGE;
			bawrite(bp);
		} else
			bdwrite(bp);
		if (u.u_offset > ip->i_size &&
		    (type == IFDIR || type == IFREG || type == IFLNK))
			ip->i_size = u.u_offset;
	} while (!u.u_error && u.u_count);
}

ino_ioctl(fp, com, data)
	struct file *fp;
	register u_int com;
	caddr_t data;
{
	register struct inode *ip = ((struct inode *)fp->f_data);
	register int fmt = ip->i_mode & IFMT;
	dev_t dev;

	switch (fmt) {

	case IFREG:
	case IFDIR:
		if (com == FIONREAD) {
			if (fp->f_type==DTYPE_PIPE && !(fp->f_flag&FREAD))
				*(off_t *)data = 0;
			else
				*(off_t *)data = ip->i_size - fp->f_offset;
			return (0);
		}
		if (com == FIONBIO || com == FIOASYNC)	/* XXX */
			return (0);			/* XXX */
		/* fall into ... */

	default:
		return (ENOTTY);

	case IFCHR:
		dev = ip->i_rdev;
		u.u_r.r_val1 = 0;
		if (setjmp(&u.u_qsave)) {
			if ((u.u_sigintr & sigmask(u.u_procp->p_cursig)) != 0)
				return(EINTR);
			u.u_eosys = RESTARTSYS;
			return (0);
		}
		return ((*cdevsw[major(dev)].d_ioctl)(dev, com, data,
		    fp->f_flag));
	}
}

ino_select(fp, which)
	struct file *fp;
	int which;
{
	register struct inode *ip = (struct inode *)fp->f_data;
	register dev_t dev;

	switch (ip->i_mode & IFMT) {

	default:
		return (1);		/* XXX */

	case IFCHR:
		dev = ip->i_rdev;
		return (*cdevsw[major(dev)].d_select)(dev, which);
	}
}

ino_stat(ip, sb)
	register struct inode *ip;
	register struct stat *sb;
{

	ITIMES(ip, &time, &time);
	/*
	 * Copy from inode table
	 */
	sb->st_dev = ip->i_dev;
	sb->st_ino = ip->i_number;
	sb->st_mode = ip->i_mode;
	sb->st_nlink = ip->i_nlink;
	sb->st_uid = ip->i_uid;
	sb->st_gid = ip->i_gid;
	sb->st_rdev = (dev_t)ip->i_rdev;
	sb->st_size = ip->i_size;
	sb->st_atime = ip->i_atime;
	sb->st_spare1 = 0;
	sb->st_mtime = ip->i_mtime;
	sb->st_spare2 = 0;
	sb->st_ctime = ip->i_ctime;
	sb->st_spare3 = 0;
	sb->st_blksize = MAXBSIZE;
	/*
	 * blocks are too tough to do; it's not worth the effort.
	 */
	sb->st_blocks = btodb(ip->i_size + MAXBSIZE - 1);
	sb->st_spare4[0] = sb->st_spare4[1] = 0;
	return (0);
}

ino_close(fp)
	register struct file *fp;
{
	register struct inode *ip = (struct inode *)fp->f_data;
	register struct mount *mp;
	int flag, mode;
	dev_t dev;
	int (*cfunc)();

	if (fp->f_flag & (FSHLOCK|FEXLOCK))
		ino_unlock(fp, FSHLOCK|FEXLOCK);
	flag = fp->f_flag;
	dev = (dev_t)ip->i_rdev;
	mode = ip->i_mode & IFMT;
	ilock(ip);
	if (fp->f_type == DTYPE_PIPE) {
		ip->i_mode &= ~(IREAD|IWRITE);
		wakeup((caddr_t)ip+1);
		wakeup((caddr_t)ip+2);
	}
	iput(ip);
	fp->f_data = (caddr_t) 0;		/* XXX */
	switch (mode) {

	case IFCHR:
		cfunc = cdevsw[major(dev)].d_close;
		break;

	case IFBLK:
		/*
		 * We don't want to really close the device if it is mounted
		 */
/* MOUNT TABLE SHOULD HOLD INODE */
		for (mp = mount; mp < &mount[NMOUNT]; mp++)
			if (mp->m_inodp != NULL && mp->m_dev == dev)
				return;
		cfunc = bdevsw[major(dev)].d_close;
		break;

	default:
		return;
	}
	/*
	 * Check that another inode for the same device isn't active.
	 * This is because the same device can be referenced by two
	 * different inodes.
	 */
	for (fp = file; fp < fileNFILE; fp++) {
		if (fp->f_type != DTYPE_INODE)
			continue;
		if (fp->f_count && (ip = (struct inode *)fp->f_data) &&
		    ip->i_rdev == dev && (ip->i_mode&IFMT) == mode)
			return;
	}
	if (mode == IFBLK) {
		/*
		 * On last close of a block device (that isn't mounted)
		 * we must invalidate any in core blocks, so that
		 * we can, for instance, change floppy disks.
		 */
		bflush(dev);
		binval(dev);
	}
	if (setjmp(&u.u_qsave)) {
		/*
		 * If device close routine is interrupted,
		 * must return so closef can clean up.
		 */
		if (u.u_error == 0)
			u.u_error = EINTR;	/* ??? */
		return;
	}
	(*cfunc)(dev, flag);
}

/*
 * Place an advisory lock on an inode.
 */
ino_lock(fp, cmd)
	register struct file *fp;
	int cmd;
{
	register int priority = PLOCK;
	register struct inode *ip = (struct inode *)fp->f_data;

	if ((cmd & LOCK_EX) == 0)
		priority += 4;
	if (setjmp(&u.u_qsave)) {
		if ((u.u_sigintr & sigmask(u.u_procp->p_cursig)) != 0)
			return(EINTR);
		u.u_eosys = RESTARTSYS;
		return (0);
	}
	/*
	 * If there's a exclusive lock currently applied
	 * to the file, then we've gotta wait for the
	 * lock with everyone else.
	 */
again:
	while (ip->i_flag & IEXLOCK) {
		/*
		 * If we're holding an exclusive
		 * lock, then release it.
		 */
		if (fp->f_flag & FEXLOCK) {
			ino_unlock(fp, FEXLOCK);
			continue;
		}
		if (cmd & LOCK_NB)
			return (EWOULDBLOCK);
		ip->i_flag |= ILWAIT;
		sleep((caddr_t)&ip->i_exlockc, priority);
	}
	if ((cmd & LOCK_EX) && (ip->i_flag & ISHLOCK)) {
		/*
		 * Must wait for any shared locks to finish
		 * before we try to apply a exclusive lock.
		 *
		 * If we're holding a shared
		 * lock, then release it.
		 */
		if (fp->f_flag & FSHLOCK) {
			ino_unlock(fp, FSHLOCK);
			goto again;
		}
		if (cmd & LOCK_NB)
			return (EWOULDBLOCK);
		ip->i_flag |= ILWAIT;
		sleep((caddr_t)&ip->i_shlockc, PLOCK);
		goto again;
	}
	if (fp->f_flag & FEXLOCK)
		panic("ino_lock");
	if (cmd & LOCK_EX) {
		cmd &= ~LOCK_SH;
		ip->i_exlockc++;
		ip->i_flag |= IEXLOCK;
		fp->f_flag |= FEXLOCK;
	}
	if ((cmd & LOCK_SH) && (fp->f_flag & FSHLOCK) == 0) {
		ip->i_shlockc++;
		ip->i_flag |= ISHLOCK;
		fp->f_flag |= FSHLOCK;
	}
	return (0);
}

/*
 * Unlock a file.
 */
ino_unlock(fp, kind)
	register struct file *fp;
	int kind;
{
	register struct inode *ip = (struct inode *)fp->f_data;
	register int flags;

	kind &= fp->f_flag;
	if (ip == NULL || kind == 0)
		return;
	flags = ip->i_flag;
	if (kind & FSHLOCK) {
		if ((flags & ISHLOCK) == 0)
			panic("ino_unlock: SHLOCK");
		if (--ip->i_shlockc == 0) {
			ip->i_flag &= ~ISHLOCK;
			if (flags & ILWAIT)
				wakeup((caddr_t)&ip->i_shlockc);
		}
		fp->f_flag &= ~FSHLOCK;
	}
	if (kind & FEXLOCK) {
		if ((flags & IEXLOCK) == 0)
			panic("ino_unlock: EXLOCK");
		if (--ip->i_exlockc == 0) {
			ip->i_flag &= ~(IEXLOCK|ILWAIT);
			if (flags & ILWAIT)
				wakeup((caddr_t)&ip->i_exlockc);
		}
		fp->f_flag &= ~FEXLOCK;
	}
}

/*
 * Openi called to allow handler
 * of special files to initialize and
 * validate before actual IO.
 */
openi(ip, mode)
	register struct inode *ip;
{
	register dev_t dev = ip->i_rdev;
	register int maj = major(dev);

	switch (ip->i_mode&IFMT) {

	case IFCHR:
		if ((u_int)maj >= nchrdev)
			return (ENXIO);
		return ((*cdevsw[maj].d_open)(dev, mode));

	case IFBLK:
		if ((u_int)maj >= nblkdev)
			return (ENXIO);
		return ((*bdevsw[maj].d_open)(dev, mode));
	}
	return (0);
}
