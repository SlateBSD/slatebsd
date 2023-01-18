/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)inet.h	5.1 (Berkeley) 5/30/85
 */

/*
 * External definitions for
 * functions in inet(3N)
 */
#include <short_names.h>

u_long inet_addr();
char	*inet_ntoa();
struct	in_addr inet_makeaddr();
u_long inet_network();
u_long inet_netof();
u_long inet_lnaof();
