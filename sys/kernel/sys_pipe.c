/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)sys_pipe.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

#include "param.h"
#include "../machine/reg.h"

#include "systm.h"
#include "user.h"
#include "inode.h"
#include "file.h"
#include "fs.h"
#include "mount.h"

/*
 * The sys-pipe entry.
 * Allocate an inode on the root device.  Allocate 2
 * file structures.  Put it all together with flags.
 */
pipe()
{
	register struct inode *ip;
	register struct file *rf, *wf;
	static struct mount *mp;
	struct inode itmp;
	int r;

	/*
	 * if pipedev not yet found, or not available, get it; if can't
	 * find it, use rootdev.  It would be cleaner to wander around
	 * and fix it so that this and getfs() only check m_dev OR
	 * m_inodp, but hopefully the mount table isn't scanned enough
	 * to make it a problem.  Besides, 4.3's is just as bad.  Basic
	 * fantasy is that if m_inodp is set, m_dev *will* be okay.
	 */
	if (!mp || !mp->m_inodp || mp->m_dev != pipedev) {
		for (mp = &mount[0];;++mp) {
			if (mp == &mount[NMOUNT]) {
				mp = &mount[0];		/* use root */
				break;
			}
			if (mp->m_inodp == NULL || mp->m_dev != pipedev)
				continue;
			break;
		}
		if (mp->m_filsys.fs_ronly) {
			u.u_error = EROFS;
			return;
		}
	}
	itmp.i_fs = &mp->m_filsys;
	itmp.i_dev = mp->m_dev;
	ip = ialloc(&itmp);
	if (ip == NULL)
		return;
	rf = falloc();
	if (rf == NULL) {
		iput(ip);
		return;
	}
	r = u.u_r.r_val1;
	wf = falloc();
	if (wf == NULL) {
		rf->f_count = 0;
		u.u_ofile[r] = NULL;
		iput(ip);
		return;
	}
	u.u_r.r_val2 = u.u_r.r_val1;
	u.u_r.r_val1 = r;
	wf->f_flag = FWRITE;
	rf->f_flag = FREAD;
	rf->f_type = wf->f_type = DTYPE_PIPE;
	rf->f_data = wf->f_data = (caddr_t)ip;
	ip->i_count = 2;
	ip->i_mode = IFREG;
	ip->i_flag = IACC|IUPD|ICHG|IPIPE;
}

readp(fp)
	register struct file *fp;
{
	register struct inode *ip;

	ip = (struct inode *)fp->f_data;
loop:
	/* Very conservative locking. */
	ILOCK(ip);

	/* If nothing in the pipe, wait. */
	if (ip->i_size == 0) {
		/*
		 * If there are not both reader and writer active,
		 * return without satisfying read.
		 */
		IUNLOCK(ip);
		if (ip->i_count != 2)
			return;
		ip->i_mode |= IREAD;
		sleep((caddr_t)ip+2, PPIPE);
		goto loop;
	}

	/* Read and return */
	u.u_offset = fp->f_offset;
	readi(ip);
	fp->f_offset = u.u_offset;

	/*
	 * If reader has caught up with writer, reset
	 * offset and size to 0.
	 */
	if (fp->f_offset == ip->i_size) {
		fp->f_offset = 0;
		ip->i_size = 0;
		if (ip->i_mode & IWRITE) {
			ip->i_mode &= ~IWRITE;
			wakeup((caddr_t)ip+1);
		}
	}
	IUNLOCK(ip);
}

writep(fp)
	register struct file *fp;
{
	register struct inode *ip;
	register int c;

	ip = (struct inode *)fp->f_data;
	c = u.u_count;
loop:
	/* If all done, return. */
	ILOCK(ip);
	if (c == 0) {
		IUNLOCK(ip);
		u.u_count = 0;
		return;
	}

	/*
	 * If there are not both read and write sides of the pipe active,
	 * return error and signal too.
	 */
	if (ip->i_count != 2) {
		IUNLOCK(ip);
		u.u_error = EPIPE;
		psignal(u.u_procp, SIGPIPE);
		return;
	}

	/*
	 * If the pipe is full, wait for reads to deplete
	 * and truncate it.
	 */
	if (ip->i_size >= MAXPIPSIZ) {
		ip->i_mode |= IWRITE;
		IUNLOCK(ip);
		sleep((caddr_t)ip+1, PPIPE);
		goto loop;
	}

	/*
	 * Write what is possible and loop back.
	 * If writing less than MAXPIPSIZ, it always goes.
	 * One can therefore get a file > MAXPIPSIZ if write
	 * sizes do not divide MAXPIPSIZ.
	 */
	u.u_offset = ip->i_size;
	u.u_count = MIN((u_int)c, (u_int)MAXPIPSIZ);
	c -= u.u_count;
	writei(ip);
	IUNLOCK(ip);
	if (ip->i_mode&IREAD) {
		ip->i_mode &= ~IREAD;
		wakeup((caddr_t)ip+2);
	}
	goto loop;
}
