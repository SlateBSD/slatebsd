/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)inode.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * The I node is the focus of all file activity in UNIX.
 * There is a unique inode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 * An inode is 'named' by its dev/inumber pair. (iget/iget.c)
 * Data in icommon1 and icommon2 is read in from permanent inode on volume.
 */

/*
 * 21 of the di_addr address bytes are used; 7 addresses of 3
 * bytes each: 4 direct (4Kb directly accessible) and 3 indirect.
 */
#define	NDADDR	4			/* direct addresses in inode */
#define	NIADDR	3			/* indirect addresses in inode */
#define	NADDR	(NDADDR + NIADDR)	/* total addresses in inode */

struct inode {
	struct	inode *i_chain[2];	/* must be first */
	u_short	i_flag;
	u_short	i_count;	/* reference count */
	dev_t	i_dev;		/* device where inode resides */
	u_short	i_shlockc;	/* count of shared locks on inode */
	u_short	i_exlockc;	/* count of exclusive locks on inode */
	ino_t	i_number;	/* i number, 1-to-1 with device address */
	struct	fs *i_fs;	/* file sys associated with this inode */
	struct	text *i_text;	/* text entry, if any (should be region) */
	union {
		daddr_t	I_addr[NADDR];		/* normal file/directory */
		struct {
			daddr_t	I_db[NDADDR];	/* normal file/directory */
			daddr_t	I_ib[NIADDR];
		} i_f;
		struct {
			/*
			 * the dummy field is here so that the de/compression
			 * part of the iget/iput routines works for special
			 * files.
			 */
			u_short	I_dummy;
			dev_t	I_rdev;		/* dev type */
		} i_d;
	} i_un1;
	union {
		daddr_t	if_lastr;	/* last read (read-ahead) */
#ifdef UCB_NET
		struct	socket *is_socket;
#endif
		struct	{
			struct inode  *if_freef;	/* free list forward */
			struct inode **if_freeb;	/* free list back */
		} i_fr;
	} i_un2;
	struct icommon1 {
		u_short	ic_mode;	/* mode and type of file */
		u_short	ic_nlink;	/* number of links to file */
		uid_t	ic_uid;		/* owner's user id */
		gid_t	ic_gid;		/* owner's group id */
		off_t	ic_size;	/* number of bytes in file */
	} i_ic1;
	struct icommon2 {
		time_t	ic_atime;	/* time last accessed */
		time_t	ic_mtime;	/* time last modified */
		time_t	ic_ctime;	/* time created */
	} i_ic2;
};

/*
 * Inode structure as it appears on
 * a disk block.
 */
struct dinode {
	struct	icommon1 di_icom1;
	char	di_addr[40];		/* disk block addresses */
	struct	icommon2 di_icom2;
};

#define	i_mode		i_ic1.ic_mode
#define	i_nlink		i_ic1.ic_nlink
#define	i_uid		i_ic1.ic_uid
#define	i_gid		i_ic1.ic_gid
#define	i_size		i_ic1.ic_size
#define	i_db		i_un1.i_f.I_db
#define	i_ib		i_un1.i_f.I_ib
#define	i_atime		i_ic2.ic_atime
#define	i_mtime		i_ic2.ic_mtime
#define	i_ctime		i_ic2.ic_ctime
#define	i_rdev		i_un1.i_d.I_rdev
#define	i_lastr		i_un2.if_lastr
#define	i_socket	i_un2.is_socket
#define	i_forw		i_chain[0]
#define	i_back		i_chain[1]
#define	i_freef		i_un2.i_fr.if_freef
#define	i_freeb		i_un2.i_fr.if_freeb
#define	i_addr		i_un1.I_addr
#define	i_dummy		i_un1.i_d.I_dummy

#define di_ic1		di_icom1
#define di_ic2		di_icom2
#define	di_mode		di_ic1.ic_mode
#define	di_nlink	di_ic1.ic_nlink
#define	di_uid		di_ic1.ic_uid
#define	di_gid		di_ic1.ic_gid
#define	di_size		di_ic1.ic_size
#define	di_atime	di_ic2.ic_atime
#define	di_mtime	di_ic2.ic_mtime
#define	di_ctime	di_ic2.ic_ctime

#ifdef KERNEL
struct inode inode[];		/* the inode table itself */
struct inode *inodeNINODE;	/* the end of the inode table */
int	ninode;			/* the number of slots in the table */

struct	inode *rootdir;			/* pointer to inode of root directory */

struct	inode *ialloc();
struct	inode *iget();
struct	inode *owner();
struct	inode *maknode();
struct	inode *namei();
#endif

/* flags */
#define	ILOCKED		0x1		/* inode is locked */
#define	IUPD		0x2		/* file has been modified */
#define	IACC		0x4		/* inode access time to be updated */
#define	IMOUNT		0x8		/* inode is mounted on */
#define	IWANT		0x10		/* some process waiting on lock */
#define	ITEXT		0x20		/* inode is pure text prototype */
#define	ICHG		0x40		/* inode has been changed */
#define	ISHLOCK		0x80		/* file has shared lock */
#define	IEXLOCK		0x100		/* file has exclusive lock */
#define	ILWAIT		0x200		/* someone waiting on file lock */
#define	IMOD		0x400		/* inode has been modified */
#define	IRENAME		0x800		/* inode is being renamed */
#define	IPIPE		0x1000		/* inode is a pipe */
#define	IXMOD		0x8000		/* inode is text, but impure (XXX) */

/* modes */
#define	IFMT		0170000		/* type of file */
#define	IFCHR		0020000		/* character special */
#define	IFDIR		0040000		/* directory */
#define	IFBLK		0060000		/* block special */
#define	IFREG		0100000		/* regular */
#define	IFLNK		0120000		/* symbolic link */
#define	IFSOCK		0140000		/* socket */
#define	ISUID		04000		/* set user id on execution */
#define	ISGID		02000		/* set group id on execution */
#define	ISVTX		01000		/* save swapped text even after use */
#define	IREAD		0400		/* read, write, execute permissions */
#define	IWRITE		0200
#define	IEXEC		0100

#define	ILOCK(ip) { \
	while ((ip)->i_flag & ILOCKED) { \
		(ip)->i_flag |= IWANT; \
		sleep((caddr_t)(ip), PINOD); \
	} \
	(ip)->i_flag |= ILOCKED; \
}

#define	IUNLOCK(ip) { \
	(ip)->i_flag &= ~ILOCKED; \
	if ((ip)->i_flag&IWANT) { \
		(ip)->i_flag &= ~IWANT; \
		wakeup((caddr_t)(ip)); \
	} \
}

#define	IUPDAT(ip, t1, t2, waitfor) { \
	if (ip->i_flag&(IUPD|IACC|ICHG|IMOD)) \
		iupdat(ip, t1, t2, waitfor); \
}

#define	ITIMES(ip, t1, t2) { \
	if ((ip)->i_flag&(IUPD|IACC|ICHG)) { \
		(ip)->i_flag |= IMOD; \
		if ((ip)->i_flag&IACC) \
			(ip)->i_atime = (t1)->tv_sec; \
		if ((ip)->i_flag&IUPD) \
			(ip)->i_mtime = (t2)->tv_sec; \
		if ((ip)->i_flag&ICHG) \
			(ip)->i_ctime = time.tv_sec; \
		(ip)->i_flag &= ~(IACC|IUPD|ICHG); \
	} \
}
