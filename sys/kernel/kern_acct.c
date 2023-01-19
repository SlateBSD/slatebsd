/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/fs.h>
#include <sys/inode.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/acct.h>
#include <sys/namei.h>
#include <sys/kernel.h>

/*
 * SHOULD REPLACE THIS WITH A DRIVER THAT CAN BE READ TO SIMPLIFY.
 */
struct	inode *acctp;

/*
 * Perform process accounting functions.
 */
int
sysacct()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
	} *uap = (struct a *)u.u_ap;

	if (suser()) {
		if (uap->fname==NULL) {
			if (ip = acctp) {
				irele(ip);
				acctp = NULL;
			}
			return;
		}
		u.u_segflg = UIO_USERSPACE;
		u.u_dirp = uap->fname;
		ip = namei(LOOKUP | FOLLOW);
		if (ip == NULL)
			return;
		if ((ip->i_mode&IFMT) != IFREG) {
			u.u_error = EACCES;
			iput(ip);
			return;
		}
		if (ip->i_fs->fs_ronly) {
			u.u_error = EROFS;
			iput(ip);
			return;
		}
		if (acctp && (acctp->i_number != ip->i_number ||
		    acctp->i_dev != ip->i_dev))
			irele(acctp);
		acctp = ip;
		iunlock(ip);
	}
}

struct	acct acctbuf;
/*
 * On exit, write a record on the accounting file.
 */
int
acct()
{
	register struct inode *ip;
	off_t siz;

	if ((ip = acctp) == NULL)
		return;
	ilock(ip);
	bcopy(u.u_comm, acctbuf.ac_comm, sizeof(acctbuf.ac_comm));
	acctbuf.ac_utime = compress(u.u_ru.ru_utime);
	acctbuf.ac_stime = compress(u.u_ru.ru_stime);
	acctbuf.ac_etime = compress(time.tv_sec - u.u_start);
	acctbuf.ac_btime = u.u_start;
	acctbuf.ac_uid = u.u_ruid;
	acctbuf.ac_gid = u.u_rgid;
	acctbuf.ac_mem = u.u_dsize+u.u_ssize;	/* probably max */
#ifdef UCB_RUSAGE
	acctbuf.ac_io = compress(u.u_ru.ru_inblock + u.u_ru.ru_oublock);
#endif
	acctbuf.ac_tty = u.u_ttyd;
	acctbuf.ac_flag = u.u_acflag;
	siz = ip->i_size;
	u.u_offset = siz;
	u.u_base = (caddr_t)&acctbuf;
	u.u_count = sizeof(acctbuf);
	u.u_segflg = UIO_SYSSPACE;
	u.u_error = 0;
	writei(ip);
	if(u.u_error)
		ip->i_size = siz;
	iunlock(ip);
}

/*
 * Produce a pseudo-floating point representation
 * with 3 bits base-8 exponent, 13 bits fraction.
 */
int
compress(register time_t t)
{
	register exp = 0, round = 0;

	while (t >= 8192) {
		exp++;
		round = t&04L;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return((exp<<13) + t);
}
