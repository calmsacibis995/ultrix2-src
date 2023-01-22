#ifndef lint
static	char	*sccsid = "@(#)mklist.c	1.1	(ULTRIX)	1/8/85";
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
# include	<aux.h>
# include	<tree.h>
# include	<symbol.h>
# include	"globs.h"


/*
**  MKLIST
**
**	writes a list of query tree nodes in "OVQP order" --
**	that is, everything in postfix (endorder) except AND's and OR's
**	infixed (postorder) to OVQP.
**	called by call_ovqp().
*/



mklist(tree)
QTREE *tree;
{
	register int	typ;
	register QTREE 	*t;
	register int 	andor;

	t = tree;
	if (!t || (typ=t->sym.type)==TREE || typ==QLEND) 
		return;

	andor=0;
	mklist(t->left);
	if (typ==AND || typ==OR)
	{
		andor = 1;
		ovqpnod(t);
	}
	mklist(t->right);
	if (!andor)
		ovqpnod(t);
}
