#ifndef lint
static char	*sccsid = "@(#)lookup.c	1.2	(ULTRIX)	1/27/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
*
*			Modification History
*
*		David Metsky,	20-Jan-86
*
* 001	Replaced old version with BSD 4.3 version as part of upgrade
*
*	Based on:	lookup.c	5.1		6.5.85
*
*************************************************************************/

#include "whoami.h"
#include "0.h"

struct nl *disptab[077+1];

/*
 * Lookup is called to
 * find a symbol in the
 * block structure symbol
 * table and returns a pointer to
 * its namelist entry.
 */
struct nl *
lookup(s)
	register char *s;
{
	register struct nl *p;
	register struct udinfo;

	if (s == NIL) {
		nocascade();
		return (NLNIL);
	}
	p = lookup1(s);
	if (p == NLNIL) {
		derror("%s is undefined", s);
		return (NLNIL);
	}
	if (p->class == FVAR) {
		p = p->chain;
		bn--;
	}
	return (p);
}

#ifndef PI0
int	flagwas;
#endif
/*
 * Lookup1 is an internal lookup.
 * It is not an error to call lookup1
 * if the symbol is not defined.  Also
 * lookup1 will return FVARs while
 * lookup never will, thus asgnop
 * calls it when it thinks you are
 * assigning to the function variable.
 */

struct nl *
lookup1(s)
	register char *s;
{
	register struct nl *p;
#ifndef PI0
	register struct nl *q;
#endif
	register int i;

	if (s == NIL)
		return (NLNIL);
	bn = cbn;
#ifndef PI0
	/*
	 * We first check the field names
	 * of the currently active with
	 * statements (expensive since they
	 * are not hashed).
	 */
	for (p = withlist; p != NLNIL; p = p->nl_next) {
		q = p->type;
		if (q == NLNIL)
			continue;
		if (reclook(q, s) != NIL)
			/*
			 * Return the WITHPTR, lvalue understands.
			 */
			return (p);
	}
#endif
	/*
	 * Symbol table is a 64 way hash
	 * on the low bits of the character
	 * pointer value. (Simple, but effective)
	 */
	i = (int) s & 077;
	for (p = disptab[i]; p != NLNIL; p = p->nl_next)
		if (p->symbol == s && p->class != FIELD && p->class != BADUSE) {
			bn = (p->nl_block & 037);
#ifndef PI0
			flagwas = p->nl_flags;
			p->nl_flags |= NUSED;
#endif
			return (p);
		}
	return (NLNIL);
}

#ifndef PI01
nlfund(sp)
	char *sp;
{
	register struct nl *p;
	register int i;

	i = (int) sp & 077;
	for (p = disptab[i]; p != NLNIL; p = p->nl_next)
	if (p->symbol == sp && (p->nl_block & 037) == 0)
		return (nloff(p));
	return (0);
}
#endif
