#ifndef lint
static	char	*sccsid = "@(#)pull_const.c	1.1	(ULTRIX)	1/8/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
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

# include	<ingres.h>
# include	<symbol.h>
# include	<aux.h>
# include	<tree.h>
# include	"globs.h"


/*
** PULL_CONST - Detach and execute all constant clauses in the qualification.
**
**	Pull_const examines the root tree for any constant clauses.
**	If none are present then it returns TRUE. If there are any
**	constant clauses, then they are removed, executed and if
**	TRUE then pull_const returns TRUE and other wise it returns
**	FALSE.
**
**	This routine is not necessary to decomposition but rather
**	can be called as an optimization when constant clauses are
**	expected. Note that without this routine, constant clauses
**	would only be examined at the bottom level of decomposition.
**	Thus a multivar query which was true except for a constant clause
**	would look at the required cross-product space before returning.
*/

pull_const(root, buf)
QTREE	*root;
char			*buf;
{
	register QTREE	*r, *q, *s;
	QTREE		*makroot();

	s = (QTREE *) NULL;

	for (r = root; r->right->sym.type != QLEND; )
	{
		q = r;
		r = r->right;	/* r is now the AND node */

		if (r->sym.value.sym_root.lvarc == 0)
		{
			/* we have a constant clause */
			if (s == 0)
				s = makroot(buf);

			/* remove AND from root tree */
			q->right = r->right;

			/* put node into constant tree */
			r->right = s->right;
			s->right = r;

			/* fix up var counts (just for good form!) */
			r->sym.value.sym_root.rvarm = r->sym.value.sym_root.tvarc = 0;

			r = q;
		}
	}

	if (s)
	{
		/* run the constant query */
		return (execsq1(s, -1, NORESULT));
	}

	return (TRUE);
}
