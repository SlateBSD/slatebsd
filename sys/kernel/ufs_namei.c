/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ufs_namei.c	1.1 (Berkeley) 12/1/86
 */
#include "param.h"
#include "../machine/seg.h"

#include "systm.h"
#include "inode.h"
#include "fs.h"
#include "mount.h"
#include "user.h"
#include "buf.h"
#include "namei.h"

/*
 * Convert a pathname into a pointer to a locked inode.
 */
struct inode *
namei(nameiop)
	int nameiop;
{
	register char *cp;		/* pointer into pathname argument */
/* these variables refer to things which must be freed or unlocked */
	struct inode *dp = 0;		/* the directory we are searching */
	struct fs *fs;			/* file system that directory is in */
	struct buf *bp = 0;		/* a buffer of directory entries */
	struct v7direct *ep;		/* the current directory entry */
/* these variables hold information about the search for a slot */
	off_t slotoffset;		/* offset of area with free space */
/* */
	int numdirpasses;		/* strategy for directory search */
	off_t endsearch;		/* offset to end directory search */
	int nlink = 0;			/* number of symbolic links taken */
	struct inode *pdp;		/* saved dp during symlink work */
	int lockparent;
	int isdotdot;			/* != 0 if current name is ".." */
	int flag;			/* op ie, LOOKUP, CREATE, or DELETE */
	off_t enduseful;		/* pointer past last used dir slot */
	char	path[MAXPATHLEN];	/* current path */

	lockparent = nameiop & LOCKPARENT;
	flag = nameiop &~ (LOCKPARENT|NOCACHE|FOLLOW);
	/*
	 * Copy the name into the buffer.
	 */
	{
		register int error;

		if (u.u_segflg == UIO_SYSSPACE)
			error = copystr(u.u_dirp, path, MAXPATHLEN,
			    (u_int *)0);
		else
			error = copyinstr(u.u_dirp, path, MAXPATHLEN,
			    (u_int *)0);
		if (error) {
			u.u_error = error;
			return (NULL);
		}
	}

	/*
	 * Get starting directory.
	 */
	cp = path;
	if (*cp == '/') {
		while (*cp == '/')
			cp++;
		if ((dp = u.u_rdir) == NULL)
			dp = rootdir;
	} else
		dp = u.u_cdir;
	fs = dp->i_fs;
	ILOCK(dp);
	dp->i_count++;
	u.ni_endoff = 0;

	/*
	 * We come to dirloop to search a new directory.
	 * The directory must be locked so that it can be
	 * iput, and fs must be already set to dp->i_fs.
	 */
dirloop:
	/*
	 * Check accessibility of directory.
	 */
	if ((dp->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
		goto bad;
	}
	if (access(dp, IEXEC))
		goto bad;

dirloop2:
	/*
	 * Copy next component of name to u.u_dent.d_name.
	 */
	{
		register char	*tp;

		for (tp = u.u_dent.d_name; *cp != 0 && *cp != '/'; cp++) {
			if (tp == &u.u_dent.d_name[MAXNAMLEN]) {
#ifdef NO_FILE_NAME_MAPPING
				u.u_error = ENAMETOOLONG;
				goto bad;
#else
				for (; *cp != 0 && *cp != '/'; cp++);
				break;
#endif
			}
			if (*cp & 0200)
				if ((*cp&0377) == ('/'|0200) || flag != DELETE) {
					u.u_error = EINVAL;
					goto bad;
				}
			*tp++ = *cp;
		}
		while (tp < &u.u_dent.d_name[MAXNAMLEN])
			*tp++ = '\0';
	}
	isdotdot = (u.u_dent.d_name[0] == '.' &&
		u.u_dent.d_name[1] == '.' && u.u_dent.d_name[2] == '\0');

	/*
	 * Check for degenerate name (e.g. / or "")
	 * which is a way of talking about a directory,
	 * e.g. like "/." or ".".
	 */
	 if (u.u_dent.d_name[0] == '\0') {
		if (flag != LOOKUP || lockparent) {
			u.u_error = EISDIR;
			goto bad;
		}
		return (dp);
	}

	/*
	 * We now have a segment name to search for, and a directory to search.
	 */

	/*
	 * If this is the same directory that this process
	 * previously searched, pick up where we last left off.
	 * We cache only lookups as these are the most common
	 * and have the greatest payoff. Caching CREATE has little
	 * benefit as it usually must search the entire directory
	 * to determine that the entry does not exist. Caching the
	 * location of the last DELETE has not reduced profiling time
	 * and hence has been removed in the interest of simplicity.
	 */
	if (flag != LOOKUP || dp->i_number != u.u_ncache.nc_inumber ||
		dp->i_dev != u.u_ncache.nc_dev) {
			u.u_offset = 0;
			numdirpasses = 1;
	} else {
		register int entryoffsetinblock;

		if (u.u_ncache.nc_prevoffset > dp->i_size)
			u.u_ncache.nc_prevoffset = 0;
		u.u_offset = u.u_ncache.nc_prevoffset;
		entryoffsetinblock = blkoff(u.u_offset);
		if (entryoffsetinblock != 0) {
			bp = bread(dp->i_dev,
			   bmap(dp,lblkno(u.u_offset),B_READ));
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				bp = NULL;
				goto bad;
			}
			ep = (struct v7direct *)(mapin(bp) +
			    entryoffsetinblock);
		}
		numdirpasses = 2;
	}
	endsearch = dp->i_size;
	enduseful = 0;
	slotoffset = -1;

searchloop:
	while (u.u_offset < endsearch) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */
		if (blkoff(u.u_offset) == 0) {
			if (bp != NULL) {
				mapout(bp);
				brelse(bp);
			}
			bp = bread(dp->i_dev,
			    bmap(dp,lblkno(u.u_offset),B_READ));
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				bp = NULL;
				goto bad;
			}
			ep = (struct v7direct *)mapin(bp);
		}

		/*
		 * Check for a name match, requires word alignment.
		 * Also note the offset of the first empty slot in
		 * slotoffset for possible create.
		 */
		if (ep->d_ino) {
			register short	*p1 = (short *)u.u_dent.d_name,
					*p2 = (short *)ep->d_name;

			if (*p1++ == *p2++ && *p1++ == *p2++ &&
			    *p1++ == *p2++ && *p1++ == *p2++ &&
			    *p1++ == *p2++ && *p1++ == *p2++ &&
			    *p1++ == *p2++)
				goto found;
		}
		else if (slotoffset == -1)
			slotoffset = u.u_offset;
		u.u_offset += sizeof(struct v7direct);
		if (ep->d_ino)
			enduseful = u.u_offset;
		++ep;
	}
/* notfound: */
	/*
	 * If we started in the middle of the directory and failed
	 * to find our target, we must check the beginning as well.
	 */
	if (numdirpasses == 2) {
		numdirpasses--;
		u.u_offset = 0;
		endsearch = u.u_ncache.nc_prevoffset;
		goto searchloop;
	}
	/*
	 * If creating, and at end of pathname and current
	 * directory has not been removed, then can consider
	 * allowing file to be created.
	 */
	if (flag == CREATE && *cp == '\0' && dp->i_nlink != 0) {
		/*
		 * Access for write is interpreted as allowing
		 * creation of files in the directory.
		 */
		if (access(dp, IWRITE))
			goto bad;
		/*
		 * Return an indication of where the new directory
		 * entry should be put.  If we didn't find a slot,
		 * then u.u_offset is already correct.  If we found
		 * a slot, then set u.u_offset to reflect it.
		 */
		if (slotoffset == -1)
			u.ni_endoff = 0;
		else {
			u.u_offset = slotoffset;
			if (enduseful < slotoffset + sizeof(struct v7direct))
				u.ni_endoff =
				    slotoffset + sizeof(struct v7direct);
			else
				u.ni_endoff = enduseful;
		}
		dp->i_flag |= IUPD|ICHG;
		if (bp) {
			mapout(bp);
			brelse(bp);
		}
		/*
		 * We return with the directory locked, so that
		 * the parameters we set up above will still be
		 * valid if we actually decide to do a direnter().
		 * We return NULL to indicate that the entry doesn't
		 * currently exist, leaving a pointer to the (locked)
		 * directory inode in u.u_pdir.
		 */
		u.u_pdir = dp;
		return (NULL);
	}
	u.u_error = ENOENT;
	goto bad;
found:
	/*
	 * Found component in pathname.
	 * If the final component of path name, save information
	 * in the cache as to where the entry was found.
	 */
	if (*cp == '\0' && flag == LOOKUP) {
		u.u_ncache.nc_prevoffset = u.u_offset;
		u.u_ncache.nc_inumber = dp->i_number;
		u.u_ncache.nc_dev = dp->i_dev;
	}
	u.u_dent = *ep;
	mapout(bp);
	brelse(bp);
	bp = NULL;

	/*
	 * If deleting, and at end of pathname, return
	 * parameters which can be used to remove file.
	 * If the lockparent flag isn't set, we return only
	 * the directory (in u.u_pdir), otherwise we go
	 * on and lock the inode, being careful with ".".
	 */
	if (flag == DELETE && *cp == 0) {
		/*
		 * Write access to directory required to delete files.
		 */
		if (access(dp, IWRITE))
			goto bad;
		u.u_pdir = dp;		/* for dirremove() */
		/*
		 * Return pointer to current entry in u.u_offset.
		 * Save directory inode pointer in u.u_pdir for dirremove().
		 */
		if (lockparent) {
			if (dp->i_number == u.u_dent.d_ino)
				dp->i_count++;
			else {
				dp = iget(dp->i_dev, fs, u.u_dent.d_ino);
				if (dp == NULL) {
					iput(u.u_pdir);
					goto bad;
				}
				/*
				 * If directory is "sticky", then user must own
				 * the directory, or the file in it, else he
				 * may not delete it (unless he's root). This
				 * implements append-only directories.
				 */
				if ((u.u_pdir->i_mode & ISVTX) &&
				    u.u_uid != 0 &&
				    u.u_uid != u.u_pdir->i_uid &&
				    dp->i_uid != u.u_uid) {
					iput(u.u_pdir);
					u.u_error = EPERM;
					goto bad;
				}
			}
		}
		return (dp);
	}

	/*
	 * Special handling for ".." allowing chdir out of mounted
	 * file system: indirect .. in root inode to reevaluate
	 * in directory file system was mounted on.
	 */
	if (isdotdot) {
		if (dp == u.u_rdir)
			u.u_dent.d_ino = dp->i_number;
		else if (u.u_dent.d_ino == ROOTINO &&
		    dp->i_number == ROOTINO) {
			register struct mount *mp;
			register dev_t d;

			d = dp->i_dev;
			for (mp = &mount[1]; mp < &mount[NMOUNT]; mp++)
				if (mp->m_inodp && mp->m_dev == d) {
					iput(dp);
					dp = mp->m_inodp;
					ILOCK(dp);
					dp->i_count++;
					fs = dp->i_fs;
					cp -= 2;	/* back over .. */
					goto dirloop2;
				}
		}
	}

	/*
	 * If rewriting (rename), return the inode and the
	 * information required to rewrite the present directory
	 * Must get inode of directory entry to verify it's a
	 * regular file, or empty directory.  
	 */
	if ((flag == CREATE && lockparent) && *cp == 0) {
		if (access(dp, IWRITE))
			goto bad;
		u.u_pdir = dp;		/* for dirrewrite() */
		/*
		 * Careful about locking second inode. 
		 * This can only occur if the target is ".". 
		 */
		if (dp->i_number == u.u_dent.d_ino) {
			u.u_error = EISDIR;		/* XXX */
			goto bad;
		}
		dp = iget(dp->i_dev, fs, u.u_dent.d_ino);
		if (dp == NULL) {
			iput(u.u_pdir);
			goto bad;
		}
		return (dp);
	}

	/*
	 * Check for symbolic link, which may require us to massage the
	 * name before we continue translation.  We do not `iput' the
	 * directory because we may need it again if the symbolic link
	 * is relative to the current directory.  Instead we save it
	 * unlocked as "pdp".  We must get the target inode before unlocking
	 * the directory to insure that the inode will not be removed
	 * before we get it.  We prevent deadlock by always fetching
	 * inodes from the root, moving down the directory tree. Thus
	 * when following backward pointers ".." we must unlock the
	 * parent directory before getting the requested directory.
	 * There is a potential race condition here if both the current
	 * and parent directories are removed before the `iget' for the
	 * inode associated with ".." returns.  We hope that this occurs
	 * infrequently since we cannot avoid this race condition without
	 * implementing a sophisticated deadlock detection algorithm.
	 * Note also that this simple deadlock detection scheme will not
	 * work if the file system has any hard links other than ".."
	 * that point backwards in the directory structure.
	 */
	pdp = dp;
	if (isdotdot) {
		IUNLOCK(pdp);	/* race to get the inode */
		dp = iget(dp->i_dev, dp->i_fs, u.u_dent.d_ino);
		if (dp == NULL)
			goto bad2;
	} else if (dp->i_number == u.u_dent.d_ino) {
		dp->i_count++;	/* we want ourself, ie "." */
	} else {
		dp = iget(dp->i_dev, dp->i_fs, u.u_dent.d_ino);
		IUNLOCK(pdp);
		if (dp == NULL)
			goto bad2;
	}

	fs = dp->i_fs;

	/*
	 * Check for symbolic link
	 */
	if ((dp->i_mode & IFMT) == IFLNK &&
	    ((nameiop & FOLLOW) || *cp == '/')) {
		register int pathlen = strlen(cp) + 1;

		if (dp->i_size + pathlen >= MAXPATHLEN - 1) {
			u.u_error = ENAMETOOLONG;
			goto bad2;
		}
		if (++nlink > MAXSYMLINKS) {
			u.u_error = ELOOP;
			goto bad2;
		}

		bp = bread(dp->i_dev, bmap(dp, (daddr_t)0, B_READ));
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			bp = NULL;
			goto bad2;
		}
		/*
		 * Shift the rest of path further down the buffer, then
		 * copy link path into the first part of the buffer.
		 */
		bcopy(cp, path + dp->i_size, pathlen);
		bcopy(mapin(bp), path, (u_int)dp->i_size);
		mapout(bp);
		brelse(bp);
		bp = NULL;
		cp = path;
		iput(dp);
		if (*cp == '/') {
			irele(pdp);
			while (*cp == '/')
				cp++;
			if ((dp = u.u_rdir) == NULL)
				dp = rootdir;
			ILOCK(dp);
			dp->i_count++;
		} else {
			dp = pdp;
			ILOCK(dp);
		}
		fs = dp->i_fs;
		goto dirloop;
	}

	/*
	 * Not a symbolic link.  If more pathname,
	 * continue at next component, else return.
	 */
	if (*cp == '/') {
		while (*cp == '/')
			cp++;
		irele(pdp);
		goto dirloop;
	}
	if (lockparent)
		u.u_pdir = pdp;
	else
		irele(pdp);
	return (dp);
bad2:
	irele(pdp);
bad:
	if (bp) {
		mapout(bp);
		brelse(bp);
	}
	if (dp)
		iput(dp);
	return (NULL);
}

/*
 * Write a directory entry with parameters left as side effects
 * to a call to namei.
 */
direnter(ip)
	struct inode *ip;
{
#ifdef DIAGNOSTIC
	/*
	 * There's no reason for this to be here that I can think of.
	 * KB
	 */
	if (!u.u_pdir->i_nlink) {
		panic("direnter");
	/*
		iput(u.u_pdir);
		return(ENOTDIR);
	*/
	}
#endif
	u.u_dent.d_ino = ip->i_number;
	u.u_count = sizeof(struct v7direct);
	u.u_segflg = UIO_SYSSPACE;
	u.u_base = (caddr_t)&u.u_dent;
	writei(u.u_pdir);
	if (u.ni_endoff &&
	    u.u_pdir->i_size - u.ni_endoff > sizeof(struct v7direct) * 10)
		itrunc(u.u_pdir, (u_long)u.ni_endoff);
	iput(u.u_pdir);
	return(u.u_error);
}

/*
 * Remove a directory entry after a call to namei, using the
 * parameters which it left in the u. area.  The u. entry
 * u_offset contains the offset into the directory of the
 * entry to be eliminated.
 */
dirremove()
{
	u.u_dent.d_ino = 0;
	u.u_count = sizeof(struct v7direct);
	u.u_segflg = UIO_SYSSPACE;
	u.u_base = (caddr_t)&u.u_dent;
	writei(u.u_pdir);
	return(u.u_error);
}

/*
 * Rewrite an existing directory entry to point at the inode
 * supplied.  The parameters describing the directory entry are
 * set up by a call to namei.
 */
dirrewrite(dp, ip)
	register struct inode *dp;
	struct inode *ip;
{
	u.u_dent.d_ino = ip->i_number;
	u.u_count = sizeof(struct v7direct);
	u.u_segflg = UIO_SYSSPACE;
	u.u_base = (caddr_t)&u.u_dent;
	writei(dp);
	iput(dp);
}

/*
 * Check if a directory is empty or not.
 * Inode supplied must be locked.
 *
 * NB: does not handle corrupted directories.
 */
dirempty(ip, parentino)
	register struct inode *ip;
	ino_t parentino;
{
	register struct buf *bp = 0;	/* a buffer of directory entries */
	register struct v7direct *dp;	/* the current directory entry */
	off_t off;			/* offset into directory */

	for (off = 0;off < ip->i_size;off += sizeof(struct v7direct),++dp) {
		if (blkoff(off) == 0) {
			if (bp != NULL) {
				mapout(bp);
				brelse(bp);
			}
			bp = bread(ip->i_dev, bmap(ip,lblkno(off),B_READ));
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				return(0);
			}
			dp = (struct v7direct *)mapin(bp);
		}
		/* skip empty entries */
		if (dp->d_ino) {
			/* accept only "." and ".." */
			if (dp->d_name[2] || dp->d_name[0] != '.')
				break;
			/*
			 * At this point name length must be 1 or 2.
			 * 1 implies ".", 2 implies ".." if second
			 * char is also "."
			 */
			if (dp->d_name[1] &&
			    (dp->d_name[1] != '.' || dp->d_ino != parentino))
					break;
		}
	}
	if (bp != NULL) {
		mapout(bp);
		brelse(bp);
	}
	return(off >= ip->i_size);
}

/*
 * Check if source directory is in the path of the target directory.
 * Target is supplied locked, source is unlocked.
 * The target is always iput() before returning.
 */
checkpath(source, target)
	struct inode *source, *target;
{
	struct dirtemplate dirbuf;
	register struct inode *ip;
	register int error = 0;

	ip = target;
	if (ip->i_number == source->i_number) {
		error = EEXIST;
		goto out;
	}
	if (ip->i_number == ROOTINO)
		goto out;

	u.u_segflg = UIO_SYSSPACE;
	for (;;) {
		if ((ip->i_mode&IFMT) != IFDIR) {
			error = ENOTDIR;
			break;
		}
		u.u_base = (caddr_t)&dirbuf;
		u.u_count = sizeof(struct dirtemplate);
		u.u_offset = 0;
		readi(ip);
		if (u.u_error != 0) {
			error = u.u_error;
			break;
		}
		if (dirbuf.dotdot_name[2] ||
		    dirbuf.dotdot_name[0] != '.' ||
		    dirbuf.dotdot_name[1] != '.') {
			error = ENOTDIR;
			break;
		}
		if (dirbuf.dotdot_ino == source->i_number) {
			error = EINVAL;
			break;
		}
		if (dirbuf.dotdot_ino == ROOTINO)
			break;
		iput(ip);
		ip = iget(ip->i_dev, ip->i_fs, dirbuf.dotdot_ino);
		if (ip == NULL) {
			error = u.u_error;
			break;
		}
	}
out:
	if (error == ENOTDIR)
		printf("checkpath: .. not a directory\n");
	if (ip != NULL)
		iput(ip);
	return (error);
}
