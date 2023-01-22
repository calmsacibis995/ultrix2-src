#ifndef lint
static char *sccsid = "@(#)uipc_mbuf.c	1.11	ULTRIX	10/3/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************
 *			Modification History				*
 *	Larry Cohen - 08/05/86						*
 *		- make mclrefcnt a global pointer and allocate space 	*
 *		  in mbinit.   						*
 *		- the number pages reserved is now a function of 	*
 *			maxusers.					*
 *	Jeff Chase - 03/12/86						*
 *		Added routine mclgetx to create a type 2 mbuf		*
 *		Changed m_copy to handle type 2 mbufs			*
 *	Larry Cohen  -  02/07/86					*
 *		in mcopy use M_DONTWAIT because mcopy can be called	*
 *		from an interrupt routine.				*
 *	R. Rodriguez -  11/11/85					*
 *		Add 43bsd beta tape fixes!				*
 *	Larry Cohen  -	09/16/85					*
 * 		Add 43bsd alpha tape changes  				*
 *									*
 ************************************************************************/

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/cmap.h"
#include "../h/map.h"
#include "../h/mbuf.h"
#include "../h/vm.h"
#include "../h/kernel.h"
#include "../h/kmalloc.h"


char *mclrefcnt;

mbinit()
{
	register int s;
	extern int maxusers;
	int clusterbytes;
	int factor = 1;

	s = splimp();
	if ((mclrefcnt = (char *)km_alloc(nmbclusters, KM_CLRSG)) == 0) 
		panic("mclrefcnt kmalloc failed\n");
	if (maxusers/32)
		factor =  maxusers/32;
	clusterbytes = 8*4096 * factor;
	if (m_clalloc(4096/CLBYTES, MPG_MBUFS, M_DONTWAIT) == 0)
		goto bad;
	if (m_clalloc(clusterbytes/CLBYTES, MPG_CLUSTERS, M_DONTWAIT) == 0)
		goto bad;
	splx(s);
	return;
bad:
	panic("mbinit");
}

caddr_t
m_clalloc(ncl, how, canwait)	/* must be called at splimp since rmallocing */
	register int ncl;
	register int how;
	int canwait;
{
	register int npg, mbx;
	register struct mbuf *m;
	register int i;

	npg = ncl * CLSIZE;
	mbx = rmalloc(mbmap, (long)npg);
	if (mbx == 0) {
		if (canwait == M_WAIT)
			panic("out of mbuf map");
		return (0);
	}
	m = cltom(mbx / CLSIZE);
	if (memall(&Mbmap[mbx], npg, proc, CSYS) == 0) {
		rmfree(mbmap, (long)npg, (long)mbx);
		return (0);
	}
	vmaccess(&Mbmap[mbx], (caddr_t)m, npg);
	switch (how) {

	case MPG_CLUSTERS:
		for (i = 0; i < ncl; i++) {
			m->m_off = 0;
			m->m_next = mclfree;
			mclfree = m;
			m += CLBYTES / sizeof (*m);
			mbstat.m_clfree++;
		}
		mbstat.m_clusters += ncl;
		break;

	case MPG_MBUFS:
		for (i = ncl * CLBYTES / sizeof (*m); i > 0; i--) {
			m->m_off = 0;
			m->m_type = MT_DATA;
			mbstat.m_mtypes[MT_DATA]++;
			mbstat.m_mbufs++;
			(void) m_free(m);
			m++;
		}
		break;
	}
	return ((caddr_t)m);
}

m_pgfree(addr, n)
	caddr_t addr;
	int n;
{
#ifdef lint
	addr = addr; n = n;
#endif
}

m_expand(canwait)
	register int canwait;
{

	if (m_clalloc(1, MPG_MBUFS, canwait) == 0)
		goto steal;
	return (1);
steal:
	/* should ask protocols to free code */
	return (0);
}

/* NEED SOME WAY TO RELEASE SPACE */

/*
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */
struct mbuf *
m_get(canwait, type)
	register int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	return (m);
}

struct mbuf *
m_getclr(canwait, type)
	register int canwait, type;
{
	register struct mbuf *m;

	MGET(m,canwait,type);
	if (m == 0)
		return (0);
	bzero(mtod(m, caddr_t), MLEN);
	return (m);
}

struct mbuf *
m_free(m)
	register struct mbuf *m;
{
	register struct mbuf *n;

	MFREE(m, n);
	return (n);
}

/*ARGSUSED*/
struct mbuf *
m_more(canwait, type)
	register int canwait, type;
{
	register struct mbuf *m;

	while (m_expand(canwait) == 0) {
		if (canwait == M_WAIT) {
			m_want++;
			sleep((caddr_t)&mfree, PZERO - 1);
		}
		else {
			mbstat.m_drops++;
			return (NULL);
		}
	}
#define m_more(x,y) (panic("m_more"), (struct mbuf *)0)
	MGET(m, canwait, type);
#undef m_more
	return (m);
}

m_freem(m)
	register struct mbuf *m;
{
	register struct mbuf *n;
	register int s;

	if (m == NULL)
		return;
	s = splimp();
	do {
		MFREE(m, n);
	} while (m = n);
	splx(s);
}

/*
 * Mbuffer utility routines.
 */
struct mbuf *
m_copy(m, off, len)
	register struct mbuf *m;
	register int off;
	register int len;
{
	register struct mbuf *n, **np;
	struct mbuf *top;

	if (len == 0)
		return (0);
	if (off < 0 || len < 0)
		panic("m_copy1");
	while (off > 0) {
		if (m == 0)
			panic("m_copy2");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL)
				panic("m_copy3");
			break;
		}
		MGET(n, M_DONTWAIT, m->m_type);
		*np = n;
		if (n == 0)
			goto nospace;
		n->m_len = MIN(len, m->m_len - off);
		if (m->m_off > MMAXOFF && n->m_len > MLEN) {
			mcldup(m, n, off);
			n->m_off += off;
		} else
			bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			    (unsigned)n->m_len);
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	return (top);
nospace:
	m_freem(top);
	return (0);
}

m_cat(m, n)
	register struct mbuf *m, *n;
{
	while (m->m_next)
		m = m->m_next;
	while (n) {
		if (m->m_off >= MMAXOFF ||
		    m->m_off + m->m_len + n->m_len > MMAXOFF) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		    (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

m_adj(mp, len)
	register struct mbuf *mp;
	register int len;
{
	register struct mbuf *m;
	register int count;

	if ((m = mp) == NULL)
		return;
	if (len >= 0) {
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_off += len;
				break;
			}
		}
	} else {
		/*
		 * Trim from tail.  Scan the mbuf chain,
		 * calculating its length and finding the last mbuf.
		 * If the adjustment only affects this mbuf, then just
		 * adjust and return.  Otherwise, rescan and truncate
		 * after the remaining size.
		 */
		len = -len;
		count = 0;
		for (;;) {
			count += m->m_len;
			if (m->m_next == (struct mbuf *)0)
				break;
			m = m->m_next;
		}
		if (m->m_len >= len) {
			m->m_len -= len;
			return;
		}
		count -= len;
		/*
		 * Correct length for chain is "count".
		 * Find the mbuf with last data, adjust its length,
		 * and toss data from remaining mbufs on chain.
		 */
		for (m = mp; m; m = m->m_next) {
			if (m->m_len >= count) {
				m->m_len = count;
				break;
			}
			count -= m->m_len;
		}
		while (m = m->m_next)
			m->m_len = 0;
	}
}

	
struct mbuf *
m_pullup(n, len)
	register struct mbuf *n;
	register int len;
{
	register struct mbuf *m;
	register int count;

	if (len > MLEN)
		goto bad;
	MGET(m, M_DONTWAIT, n->m_type);
	if (m == 0)
		goto bad;
	m->m_len = 0;
	do {
		count = MIN(n->m_len, len);
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t)+m->m_len,
		  (unsigned)count);
		len -= count;
		m->m_len += count;
		n->m_len -= count;
		if (n->m_len)
			n->m_off += count;
		else
			n = m_free(n);
	} while (len && n);
	if (len) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}

struct mbuf *
mclgetx(fun, arg, addr, len, wait)
	int (*fun)(), arg, len, wait;
	caddr_t addr;
{
	register struct mbuf *m;

	MGET(m, wait, MT_DATA);
	if (m == 0)
		return (0);
	m->m_off = (int)addr - (int)m;
	m->m_len = len;
	m->m_cltype = 2;
	m->m_clfun = fun;
	m->m_clarg = arg;
	m->m_clswp = NULL;
	return (m);
}

static
buffree(arg)
	int arg;
{
	km_free(arg, *(int *)arg);
}

mcldup(m, n, off)
	register struct mbuf *m, *n;
	int off;
{
	register struct mbuf *p;
	register caddr_t copybuf;

	switch (m->m_cltype) {
	case 1:
		p = mtod(m, struct mbuf *);
		n->m_off = (int)p - (int)n;
		n->m_cltype = 1;
		mclrefcnt[mtocl(p)]++;
		break;
	case 2:
		copybuf = (caddr_t)km_alloc(n->m_len + sizeof(int), 0);
		* (int *) copybuf = n->m_len + sizeof (int);
		bcopy(mtod(m, caddr_t) + off, copybuf + sizeof (int),n->m_len);
		n->m_off = (int)copybuf + sizeof (int) - (int)n - off;
		n->m_cltype = 2;
		n->m_clfun = buffree;
		n->m_clarg = (int)copybuf;
		n->m_clswp = NULL;
		break;
	default:
		panic("mcldup has bad m_cltype");
	}
}
