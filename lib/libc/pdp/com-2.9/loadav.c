/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)loadav.c	2.3 (Berkeley) 1/25/87";
#endif LIBC_SCCS and not lint

/*
 * loadav(vec)
 * double vec[3];
 * Return the load average.  vec[0] gets the 1 minute load average,
 * vec[1] gets the 5 minute load average, and vec[2] gets the 15 minute
 * load average.
 */
loadav(vec)
	double vec[];
{
	short svec[3];
	int r;

	r = gldav(svec);
	if (r < 0)
		return r;
	vec[0] = ((float) svec[0])/256.0;
	vec[1] = ((float) svec[1])/256.0;
	vec[2] = ((float) svec[2])/256.0;
	return r;
}
