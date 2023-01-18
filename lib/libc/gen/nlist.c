/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)nlist.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <sys/types.h>
#include <sys/file.h>
#include <a.out.h>
#include <stdio.h>

/*
 * nlist - retreive attributes from name list (string table version)
 */
nlist(name, list)
	char *name;
	struct nlist *list;
{
	register struct nlist *q;
	struct nlist *p;
	int n, nreq;
	FILE *f;
	off_t sa;		/* symbol address */
	struct exec buf;
	struct nlist space[BUFSIZ/sizeof (struct nlist)];

	for (q = list, nreq = 0; q->n_name[0]; q++, nreq++) {
		q->n_type = 0;
		q->n_value = 0;
	}
	f = fopen(name, "r");
	if (f == NULL)
		return (-1);
	fread((char *)&buf, sizeof buf, 1, f);
	if (N_BADMAG(buf)) {
		fclose(f);
		return (-1);
	}
	sa = (long)buf.a_text + buf.a_data;
	if (buf.a_magic == A_MAGIC5 || buf.a_magic == A_MAGIC6) {
		register int ovlcnt;
		struct ovlhdr ovlbuf;

		fread((char *)&ovlbuf, sizeof(ovlbuf), 1, f);
		for (ovlcnt = 0; ovlcnt < NOVL; ovlcnt++)
			sa += ovlbuf.ov_siz[ovlcnt];
	}
	if (!(buf.a_flag & 01))
		sa *= 2;
	sa += sizeof(buf);
	if (buf.a_magic == A_MAGIC5 || buf.a_magic == A_MAGIC6)
		sa += sizeof(struct ovlhdr);
	fseek(f, sa, L_SET);
	/*
	 * u_int n;
	 * n = buf.a_syms;
	 * while (n) {
	 *
	 * we can't use the a_syms field of the exec structure
	 * because you can have more than 64K of symbols; since
	 * symbols are the last area, just read to EOF.
	 */
	for (;;) {
		if (!(n = fread((char *)space, sizeof(struct nlist),
		    BUFSIZ / sizeof(struct nlist), f)))
			break;
		for (q = space; n--; q++) {
			for (p = list; p->n_name[0]; p++) {
				register short *s1, *s2;

				if (p->n_type)
					continue;
				s1 = (short *)q->n_name;
				s2 = (short *)p->n_name;
				if (*s1++ != *s2++ || *s1++ != *s2++ ||
				    *s1++ != *s2++ || *s1++ != *s2++)
					continue;
				p->n_value = q->n_value;
				p->n_type = q->n_type;
				if (--nreq == 0)
					goto alldone;
				break;
			}
		}
	}
alldone:
	fclose(f);
	return (nreq);
}
