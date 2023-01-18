/* @(#)errlst.c	4.4 (Berkeley) 82/04/01 */
char	*sys_errlist[] = {
	"",
	"1",				/*  1 - EPERM */
	"2",		/*  2 - ENOENT */
	"3",			/*  3 - ESRCH */
	"4",		/*  4 - EINTR */
	"5",				/*  5 - EIO */
	"6",		/*  6 - ENXIO */
	"7",			/*  7 - E2BIG */
	"8",			/*  8 - ENOEXEC */
	"9",			/*  9 - EBADF */
	"10",				/* 10 - ECHILD */
	"11",			/* 11 - EAGAIN */
	"12",			/* 12 - ENOMEM */
	"13",			/* 13 - EACCES */
	"14",				/* 14 - EFAULT */
	"15",		/* 15 - ENOTBLK */
	"16",		/* 16 - EBUSY */
	"17",				/* 17 - EEXIST */
	"18",			/* 18 - EXDEV */
	"19",			/* 19 - ENODEV */
	"20",			/* 20 - ENOTDIR */
	"21",			/* 21 - EISDIR */
	"22",			/* 22 - EINVAL */
	"23",			/* 23 - ENFILE */
	"24",			/* 24 - EMFILE */
	"25",	/* 25 - ENOTTY */
	"26",			/* 26 - ETXTBSY */
	"27",			/* 27 - EFBIG */
	"28",		/* 28 - ENOSPC */
	"29",				/* 29 - ESPIPE */
	"30",		/* 30 - EROFS */
	"31",			/* 31 - EMLINK */
	"32",				/* 32 - EPIPE */

/* math software */
	"33",			/* 33 - EDOM */
	"34",			/* 34 - ERANGE */

/* quotas */
	"35",			/* 35 - EQUOT */

/* symbolic links */
	"36",	/* 36 - ELOOP */

/* non-blocking and interrupt i/o */
	"37",		/* 37 - EWOULDBLOCK */
#ifdef	UCB_NET
	"38",		/* 38 - EINPROGRESS */
	"39",	/* 39 - EALREADY */

/* ipc/network software */

	/* argument errors */
	"40",	/* 40 - ENOTSOCK */
	"41",		/* 41 - EDESTADDRREQ */
	"42",			/* 42 - EMSGSIZE */
	"43",	/* 43 - EPROTOTYPE */
	"44",		/* 44 - ENOPROTOOPT */
	"45",		/* 45 - EPROTONOSUPPORT */
	"46",		/* 46 - ESOCKTNOSUPPORT */
	"47",	/* 47 - EOPNOTSUPP */
	"48",	/* 48 - EPFNOSUPPORT */
	"",
						/* 49 - EAFNOSUPPORT */
	"50",		/* 50 - EADDRINUSE */
	"51",	/* 51 - EADDRNOTAVAIL */

	/* operational errors */
	"52",			/* 52 - ENETDOWN */
	"53",		/* 53 - ENETUNREACH */
	"54",	/* 54 - ENETRESET */
	"55",	/* 55 - ECONNABORTED */
	"56",		/* 56 - ECONNRESET */
	"57",		/* 57 - ENOBUFS */
	"58",		/* 58 - EISCONN */
	"59",		/* 59 - ENOTCONN */
	"60",	/* 60 - ESHUTDOWN */
	"61",	/* 61 - ETOOMANYREFS */
	"62",			/* 62 - ETIMEDOUT */
	"63",			/* 63 - ECONNREFUSED */
	"64",			/* 64 - ENAMETOOLONG */
	"65",				/* 65 - EHOSTDOWN */
	"66",			/* 66 - EHOSTUNREACH */
#endif	UCB_NET
};
int	sys_nerr = { sizeof sys_errlist/sizeof sys_errlist[0] };
