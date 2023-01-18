/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)dir.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

#ifndef MAXNAMLEN
#define MAXNAMLEN	14
#endif

/*
 * Actual directory structure as it appears on disk.  For use by programs
 * and routines which need to know what a directory really looks like.
 * Don't use it unless you absolutely have to.
 */
struct	v7direct {
	ino_t	d_ino;
	char	d_name[MAXNAMLEN];
};

#ifndef KERNEL
#define DIRBLKSIZ	512

struct	direct {
	u_long	d_ino;			/* inode number of entry */
	u_short	d_reclen;		/* length of this record */
	u_short	d_namlen;		/* length of string in d_name */
	char	d_name[MAXNAMLEN+1];	/* name must be no longer than this */
};

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct direct
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp) \
    ((sizeof (struct direct) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))

/*
 * Definitions for library routines operating on directories.
 */
typedef struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	char	dd_buf[DIRBLKSIZ];
	struct direct	dd_cur;
} DIR;
#ifndef NULL
#define NULL 0
#endif
extern	DIR *opendir();
extern	struct direct *readdir();
extern	long telldir();
extern	void seekdir();
#define rewinddir(dirp)	seekdir((dirp), (long)0)
extern	void closedir();
#endif

#ifdef KERNEL
/*
 * Template for manipulating directories.
 */
#define dotdot_ino	dtdt_ino
#define dotdot_name	dtdt_name
struct dirtemplate {
	ino_t	dot_ino;
	char	dot_name[MAXNAMLEN];
	ino_t	dotdot_ino;
	char	dotdot_name[MAXNAMLEN];
};
#endif
