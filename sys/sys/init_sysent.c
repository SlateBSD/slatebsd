/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)init_sysent.c	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * System call switch table.
 */

#include "param.h"
#include "systm.h"

int	nosys();

/* Unimplemented ... */
#define	readv		nosys
#define	writev		nosys

/* 1.1 processes and protection */
int	sethostid(),gethostid(),sethostname(),gethostname(),getpid();
int	fork(),rexit(),execv(),execve(),wait();
int	getuid(),setreuid(),getgid(),getgroups(),setregid(),setgroups();
int	getpgrp(),setpgrp();
int	rtp(),ucall();					/* BSD2_10 calls */

/* 1.2 memory management */
int	sbrk();
int	getpagesize();
int	lock(),phys(),fetchi(),nostk(),fperr();		/* BSD2_10 calls */

/* 1.3 signals */
int	sigvec(),sigblock(),sigsetmask(),sigpause(),sigstack(),sigreturn();
int	kill(), killpg();

/* 1.4 timing and statistics */
int	gettimeofday(),settimeofday();
int	getitimer(),setitimer();
int	adjtime();
int	gldav();					/* BSD2_10 calls */

/* 1.5 descriptors */
int	getdtablesize(),dup(),dup2(),close();
int	select(),getdopt(),setdopt(),fcntl(),flock();

/* 1.6 resource controls */
int	getpriority(),setpriority(),getrusage(),getrlimit(),setrlimit();
int	setquota(),qquota();

/* 1.7 system operation support */
int	umount(),smount();
int	sync(),reboot(),sysacct();

/* 2.1 generic operations */
int	read(),write(),readv(),writev(),ioctl();

/* 2.2 file system */
int	chdir(),chroot();
int	mkdir(),rmdir();
int	creat(),open(),mknod(),unlink(),stat(),fstat(),lstat();
int	chown(),fchown(),chmod(),fchmod(),utimes();
int	link(),symlink(),readlink(),rename();
int	lseek(),truncate(),ftruncate(),saccess(),fsync();

/* 2.3 communications */
int	socket(),bind(),listen(),accept(),connect();
int	socketpair(),sendto(),send(),recvfrom(),recv();
int	sendmsg(),recvmsg(),shutdown(),setsockopt(),getsockopt();
int	getsockname(),getpeername(),pipe();

int	umask();		/* XXX */

/* 2.4 processes */
int	ptrace();

/* 2.5 terminals */

#ifdef UCB_NET
#define ifnet(narg, name)	narg, name
#define errnet(narg, name)	narg, name
#else
int	nonet();
#define ifnet(narg, name)	0, nosys
#define errnet(narg, name)	0, nonet
#endif

/* BEGIN JUNK */
int	profil();		/* 'cuz sys calls are interruptible */
int	vhangup();		/* should just do in exit() */
#ifndef VIRUS_VFORK
#define vfork	fork
#endif
int	vfork();		/* awaiting fork w/ copy on write */
/* END JUNK */

/*
 * Reserved/unimplemented system calls in the range 0-150 inclusive
 * are reserved for use in future Berkeley releases.
 * Additional system calls implemented in vendor and other
 * redistributions should be placed in the reserved range at the end
 * of the current calls.
 */
/*
 * This table is the switch used to transfer to the appropriate routine for
 * processing a system call.  Each row contains the number of words of
 * arguments expected in registers, how many on the stack, and a pointer to
 * the routine.
 *
 * The maximum number of direct system calls is 255 since system call numbers
 * are encoded in the lower byte of the trap instruction -- see trap.c.
 */
struct sysent sysent[] = {
	1, nosys,			/*   0 = indir or out-of-range */
	1, rexit,			/*   1 = exit */
	0, fork,			/*   2 = fork */
	3, read,			/*   3 = read */
	3, write,			/*   4 = write */
	3, open,			/*   5 = open */
	1, close,			/*   6 = close */
	0, nosys,			/*   7 = old wait */
	2, creat,			/*   8 = creat */
	2, link,			/*   9 = link */
	1, unlink,			/*  10 = unlink */
	2, execv,			/*  11 = execv */
	1, chdir,			/*  12 = chdir */
	0, nosys,			/*  13 = old time */
	3, mknod,			/*  14 = mknod */
	2, chmod,			/*  15 = chmod */
	3, chown,			/*  16 = chown; now 3 args */
	0, nosys,			/*  17 = old break */
	0, nosys,			/*  18 = old stat */
	4, lseek,			/*  19 = lseek */
	0, getpid,			/*  20 = getpid */
	3, smount,			/*  21 = mount */
	1, umount,			/*  22 = umount */
	0, nosys,			/*  23 = old setuid */
	0, getuid,			/*  24 = getuid */
	0, nosys,			/*  25 = old stime */
	4, ptrace,			/*  26 = ptrace */
	0, nosys,			/*  27 = old alarm */
	0, nosys,			/*  28 = old fstat */
	0, nosys,			/*  29 = old pause */
	0, nosys,			/*  30 = old utime */
	0, nosys,			/*  31 = was stty */
	0, nosys,			/*  32 = was gtty */
	2, saccess,			/*  33 = access */
	0, nosys,			/*  34 = old nice */
	0, nosys,			/*  35 = old ftime */
	0, sync,			/*  36 = sync */
	2, kill,			/*  37 = kill */
	2, stat,			/*  38 = stat */
	0, nosys,			/*  39 = old setpgrp */
	2, lstat,			/*  40 = lstat */
	1, dup,				/*  41 = dup */
	0, pipe,			/*  42 = pipe */
	0, nosys,			/*  43 = old times */
	4, profil,			/*  44 = profil */
	0, nosys,			/*  45 = nosys */
	0, nosys,			/*  46 = old setgid */
	0, getgid,			/*  47 = getgid */
	0, nosys,			/*  48 = old sig */
	0, nosys,			/*  49 = reserved for USG */
	0, nosys,			/*  50 = reserved for USG */
	1, sysacct,			/*  51 = turn acct off/on */
	3, phys,			/*  52 = (2.9) set phys addr */
	1, lock,			/*  53 = (2.9) lock in core */
	4, ioctl,			/*  54 = ioctl */
	1, reboot,			/*  55 = reboot */
	0, nosys,			/*  56 = old mpxchan */
	2, symlink,			/*  57 = symlink */
	3, readlink,			/*  58 = readlink */
	3, execve,			/*  59 = execve */
	1, umask,			/*  60 = umask */
	1, chroot,			/*  61 = chroot */
	2, fstat,			/*  62 = fstat */
	0, nosys,			/*  63 = reserved */
	0, getpagesize,			/*  64 = getpagesize */
	0, nosys,			/*  65 = (4.3) mremap */
	0, vfork,			/*  66 = vfork */
	0, nosys,			/*  67 = (4.3) old vread */
	0, nosys,			/*  68 = (4.3) old vwrite */
	1, sbrk,			/*  69 = sbrk */
	0, nosys,			/*  70 = (4.3) sstk */
	0, nosys,			/*  71 = (4.3) mmap */
	0, nosys,			/*  72 = (4.3) old vadvise */
	0, nosys,			/*  73 = (4.3) munmap */
	0, nosys,			/*  74 = (4.3) mprotect */
	0, nosys,			/*  75 = (4.3) madvise */
	0, vhangup,			/*  76 = vhangup */
	0, nosys,			/*  77 = (4.3) old vlimit */
	0, nosys,			/*  78 = (4.3) mincore */
	2, getgroups,			/*  79 = getgroups */
	2, setgroups,			/*  80 = setgroups */
	1, getpgrp,			/*  81 = getpgrp */
	2, setpgrp,			/*  82 = setpgrp */
	3, setitimer,			/*  83 = setitimer */
	0, wait,			/*  84 = wait */
	0, nosys,			/*  85 = (4.3) swapon */
	2, getitimer,			/*  86 = getitimer */
	2, gethostname,			/*  87 = gethostname */
	2, sethostname,			/*  88 = sethostname */
	0, getdtablesize,		/*  89 = getdtablesize */
	2, dup2,			/*  90 = dup2 */
	2, getdopt,			/*  91 = getdopt */
	3, fcntl,			/*  92 = fcntl */
	5, select,			/*  93 = select */
	2, setdopt,			/*  94 = setdopt */
	1, fsync,			/*  95 = fsync */
	3, setpriority,			/*  96 = setpriority */
	errnet(3, socket),		/*  97 = socket */
	ifnet(3, connect),		/*  98 = connect */
	ifnet(3, accept),		/*  99 = accept */
	2, getpriority,			/* 100 = getpriority */
	ifnet(4, send),			/* 101 = send */
	ifnet(4, recv),			/* 102 = recv */
	1, sigreturn,			/* 103 = sigreturn */
	ifnet(3, bind),			/* 104 = bind */
	ifnet(5, setsockopt),		/* 105 = setsockopt */
	ifnet(2, listen),		/* 106 = listen */
	0, nosys,			/* 107 = (4.3) old vtimes */
	4, sigvec,			/* 108 = sigvec */
	2, sigblock,			/* 109 = sigblock */
	2, sigsetmask,			/* 110 = sigsetmask */
	2, sigpause,			/* 111 = sigpause */
	2, sigstack,			/* 112 = sigstack */
	ifnet(3, recvmsg),		/* 113 = recvmsg */
	ifnet(3, sendmsg),		/* 114 = sendmsg */
	0, nosys,			/* 115 = (4.3) vtrace */
	2, gettimeofday,		/* 116 = gettimeofday */
	2, getrusage,			/* 117 = getrusage */
	ifnet(5, getsockopt),		/* 118 = getsockopt */
	0, nosys,			/* 119 = (4.3) (vax) resuba */
	3, readv,			/* 120 = readv */
	3, writev,			/* 121 = writev */
	2, settimeofday,		/* 122 = settimeofday */
	3, fchown,			/* 123 = fchown */
	2, fchmod,			/* 124 = fchmod */
	ifnet(6, recvfrom),		/* 125 = recvfrom */
	2, setreuid,			/* 126 = setreuid */
	2, setregid,			/* 127 = setregid */
	2, rename,			/* 128 = rename */
	3, truncate,			/* 129 = truncate */
	3, ftruncate,			/* 130 = ftruncate */
	2, flock,			/* 131 = flock */
	0, nosys,			/* 132 = nosys */
	ifnet(6, sendto),		/* 133 = sendto */
	ifnet(2, shutdown),		/* 134 = shutdown */
	errnet(4, socketpair),		/* 135 = socketpair */
	2, mkdir,			/* 136 = mkdir */
	1, rmdir,			/* 137 = rmdir */
	2, utimes,			/* 138 = utimes */
	0, nosys,			/* 139 = internal (4.2 sigreturn) */
	2, adjtime,			/* 140 = adjtime */
	ifnet(3, getpeername),		/* 141 = getpeername */
	0, gethostid,			/* 142 = gethostid */
	2, sethostid,			/* 143 = sethostid */
	2, getrlimit,			/* 144 = getrlimit */
	2, setrlimit,			/* 145 = setrlimit */
	2, killpg,			/* 146 = killpg */
	0, nosys,			/* 147 = nosys */
	2, setquota,			/* 148 = quota */
	4, qquota,			/* 149 = qquota */
	ifnet(3, getsockname),		/* 150 = getsockname */
	/*
	 * Syscalls 151-180 inclusive are reserved for vendor-specific
	 * system calls.  (This includes various calls added for compatibity
	 * with other Unix variants.)
	 */

	/*
	 * BSD2_10 special calls
	 */
	1, rtp,				/* 151 = rtp */
	0, nostk,			/* 152 = nostk */
	1, fetchi,			/* 153 = fetchi */
	4, ucall,			/* 154 = ucall */
	0, fperr,			/* 155 = fperr */
	1, gldav,			/* 156 = gldav */
};

int	nsysent = sizeof (sysent) / sizeof (sysent[0]);
