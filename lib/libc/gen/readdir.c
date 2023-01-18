/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)readdir.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <sys/param.h>
#include <sys/dir.h>

/*
 * read a V7 directory entry and present it as a BSD4.X entry.
 */

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register DIR *dirp;
{
	register struct v7direct *dp;

	for (;;) {
		if (dirp->dd_loc == 0) {
			dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, 
			    DIRBLKSIZ);
			if (dirp->dd_size <= 0)
				return NULL;
		}
		if (dirp->dd_loc >= dirp->dd_size) {
			dirp->dd_loc = 0;
			continue;
		}
		dp = (struct v7direct *)(dirp->dd_buf + dirp->dd_loc);
		dirp->dd_loc += sizeof(struct v7direct);
		if (dp->d_ino == 0)
			continue;
		/*
		 * format V7 directory structure into BSD4.X
		 */
		dirp->dd_cur.d_ino = dp->d_ino;
		bcopy(dp->d_name, dirp->dd_cur.d_name, MAXNAMLEN);
						/* insure null termination */
		dirp->dd_cur.d_name[MAXNAMLEN] = '\0';
		dirp->dd_cur.d_namlen = strlen(dirp->dd_cur.d_name);
		dirp->dd_cur.d_reclen = DIRBLKSIZ;
		return (&dirp->dd_cur);
	}
}
