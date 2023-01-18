/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)newfs.c	5.2 (Berkeley) 9/11/85";
#endif not lint

/*
 * newfs: friendly front end to mkfs
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <disktab.h>

typedef struct indx {
	char	*key;
	int	value;
} INDX;

static INDX disks[] = {
	{ "rm02", 0 },
	{ "rm03", 1 },
	{ "rm05", 2 },
	{ "rp04", 3 },	/* RP04/05/06 all have the same m/n numbers */
	{ "rp05", 3 },
	{ "rp06", 3 },
	{ "rk06", 4 },	/* RK06/07 have the same m/n numbers */
	{ "rk07", 4 },
	{ "rl01", 5 },	/* RL01/02 have the same m/n numbers */
	{ "rl02", 5 },
	{ "rk05", 6 },
	{ "ra60", 7 },
	{ "ra80", 8 },
	{ "ra81", 9 },
	{ "rc25", 10 },
	{ "rx50", 11 },
	{ "rd51", 12 },
	{ "rd52-rqdx2", 13 },	/* for RD52/53 m/n numbers are based on */
	{ "rd53-rqdx2", 13 },	/* controller (RQDX2 or RQDX3), not drive */
	{ "rd52-rqdx3", 14 },	/* type */
	{ "rd53-rqdx3", 14 },
	{ "rx02", 15 },
	{ 0, 0 }
};

static INDX cpus[] = {
	{ "23", 0 },
	{ "24", 1 },
	{ "34", 2 },
	{ "40", 2 },	/* use 11/34 values */
	{ "44", 3 },
	{ "45", 4 },
	{ "53", 3 },	/* use 11/44 values */
	{ "55", 4 },	/* use 11/45 values */
	{ "60", 4 },	/* use 11/45 values */
	{ "70", 5 },
	{ "73", 3 },	/* use 11/44 values */
	{ "83", 3 },	/* use 11/44 values - "m" a little too high, but ok */
	{ "84", 5 },	/* use 11/70 values */
	{ 0, 0 }
};

#define ASIZE(a)	(sizeof(a)/sizeof(a[0]))

static struct mn {
	int	m, n;
} mn[ASIZE(disks)][ASIZE(cpus)] = {
/*		  23	    24	      34/40	44/53	  45/55	    70/84  */
/*						73/83	  60		   */
/* RM02 */	{ {11, 80}, {10, 80}, { 8, 80}, { 6, 80}, { 7, 80}, { 5, 80} },
/* RM03 */	{ {16, 80}, {15, 80}, {12, 80}, { 8, 80}, {11, 80}, { 7, 80} },
/* RM05 */	{ {16,304}, {15,304}, {12,304}, { 8,304}, {11,304}, { 7,304} },
/* RP04/05/06 */{ {11,209}, {10,209}, { 8,209}, { 6,209}, { 7,209}, { 5,209} },
/* RK06/07 */	{ { 8, 33}, { 7, 33}, { 6, 33}, { 4, 33}, { 5, 33}, { 3, 33} },
/* RL01/02 */	{ { 7, 10}, { 6, 10}, { 6, 10}, { 4, 10}, { 5, 10}, { 3, 10} },
/* RK05 */	{ { 4, 12}, { 4, 12}, { 3, 12}, { 2, 12}, { 3, 12}, { 2, 12} },
/* RA60 */	{ {21, 84}, {21, 84}, {17, 84}, {12, 84}, {15, 84}, {10, 84} },
/* RA80 */	{ {16,217}, {16,217}, {13,217}, { 9,217}, {11,217}, { 7,217} },
/* RA81 */	{ {26,357}, {26,357}, {21,357}, {14,357}, {18,357}, {12,357} },
/* RC25 */	{ {15, 31}, {15, 31}, {13, 31}, { 9, 31}, {11, 31}, { 7, 31} },
/* RX50 */	{ { 1,  5}, { 1,  5}, { 1,  5}, { 1,  5}, { 1,  5}, { 1,  5} },
/* RD51 */	{ { 1, 36}, { 1, 36}, { 1, 36}, { 1, 36}, { 1, 36}, { 1, 36} },
/* RQDX2 */	{ { 2, 36}, { 2, 36}, { 2, 36}, { 2, 36}, { 2, 36}, { 2, 36} },
/* RQDX3 */	{ { 7, 36}, { 7, 36}, { 7, 36}, { 7, 36}, { 7, 36}, { 7, 36} },
/* RX02 */	{ { 1,  7}, { 1,  7}, { 1,  7}, { 1,  7}, { 1,  7}, { 1,  7} },
};

main(argc, argv)
	int	argc;
	char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	register struct disktab	*dp;
	register struct partition	*pp;
	register INDX	*step;
	register char	*cp;
	struct stat	st;
	long	fssize;
	int	disk, cpu, ch, just_looking, status;
	char	device[MAXPATHLEN], cmd[BUFSIZ],
		*index(), *rindex();

	just_looking = 0;
	while ((ch = getopt(argc,argv,"Nv")) != EOF)
		switch((char)ch) {
		case 'N':
		case 'v':
			++just_looking;
			break;
		case '?':
		default:
			usage();
		}

	argc -= optind;
	argv += optind;
	if (argc != 3)
		usage();

	/* check for legal disk and cpu type, get offsets into M/N array */
	for (step = disks; step->key; ++step)
		if (!strcmp(argv[1], step->key)) {
			disk = step->value;
			break;
		}
	if (!step->key) {
		fprintf(stderr, "newfs: unknown disk type %s.\n", argv[1]);
		usage();
	}
	for (step = cpus; step->key; ++step)
		if (!strcmp(argv[2], step->key)) {
			cpu = step->value;
			break;
		}
	if (!step->key) {
		fprintf(stderr, "newfs: unknown cpu type %s.\n", argv[2]);
		usage();
	}

	/* figure out device name */
	cp = rindex(argv[0], '/');
	if (cp)
		++cp;
	else
		cp = argv[0];
	if (*cp == 'r' && cp[1] != 'a' && cp[1] != 'b')
		++cp;
	sprintf(device, "/dev/r%s", cp);

	/* see if it exists and of a legal type */
	if (stat(device, &st)) {
		fprintf(stderr, "newfs: ");
		perror(device);
		exit(1);
	}
	if ((st.st_mode & S_IFMT) != S_IFCHR) {
		fprintf(stderr, "newfs: %s: not a character device.\n", device);
		exit(1);
	}

	/* see if disk is in disktab table */
	dp = getdiskbyname(argv[1]);
	if (dp == 0) {
		fprintf(stderr, "newfs: %s: unknown disk type.\n", argv[1]);
		exit(1);
	}

	/* grab partition letter */
	cp = index(argv[0], '\0') - 1;
	if (cp == 0 || *cp < 'a' || *cp > 'h') {
		fprintf(stderr, "newfs: %s: can't figure out file system partition.\n", argv[0]);
		exit(1);
	}

	/* get default partition size */
	pp = &dp->d_partitions[*cp - 'a'];
	fssize = pp->p_size;
	if (fssize < 0) {
		fprintf(stderr, "newfs: %s: no default size for `%c' partition.\n", argv[1], *cp);
		exit(1);
	}
	fssize /= 2;	/* convert from sectors to logical blocks */

	/* build command */
	sprintf(cmd, "/etc/mkfs %s %ld %d %d",
	    device, fssize, mn[disk][cpu].m, mn[disk][cpu].n);
	printf("newfs: %s\n", cmd);
	if (just_looking)
		exit(0);
	if (status = system(cmd))
		exit(status >> 8);
	if (*cp == 'a')
		fputs("newfs: don't forget to install a deadstart block.\n", stderr);
	exit(0);
}

static
usage()
{
	register INDX	*off;
	register int	cnt, len;

	fputs("usage: newfs [ -N ] special-device device-type cpu\n", stderr);
	fputs("Known device-types are:\n\t", stderr);
	for (cnt = 8, off = disks;;) {
		len = strlen(off->key) + 2;
		if ((cnt += len) > 50) {
			fputs("\n\t", stderr);
			cnt = len + 8;
		}
		fputs(off->key, stderr);
		if ((++off)->key)
			fputs(", ", stderr);
		else
			break;
	}
	fputs("\n\nKnown cpu types are:\n\t", stderr);
	for (cnt = 8, off = cpus;;) {
		len = strlen(off->key) + 2;
		if ((cnt += len) > 50) {
			fputs("\n\t", stderr);
			cnt = len + 8;
		}
		fputs(off->key, stderr);
		if ((++off)->key)
			fputs(", ", stderr);
		else
			break;
	}
	putc('\n', stderr);
	exit(1);
}
