/*
 * void
 * ldfps(nfps);
 *	u_int nfps;
 *
 * Load fps with nfps.
 */
#include "SYS.h"

ldfps	= 170100^tst

ENTRY(ldfps)
	ldfps	2(sp)		/ load fps with new configuration
	rts	pc		/ (no value returned)
