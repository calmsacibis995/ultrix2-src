#ifndef lint
static	char	*sccsid = "@(#)tree.c	1.1	(ULTRIX)	1/8/85";
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
# include	<tree.h>
# include	<symbol.h>
# include	"qrymod.h"


/*
**  TREE -- create new tree node.
**
**	This is a stripped down version of the same thing in the
**	parser.
**
**	It only knows about lengths of zero and two.
**
**	Parameters:
**		lptr -- the left pointer.
**		rptr -- the right pointer.
**		typ -- the node type.
**		len -- the node length.
**		value -- the node value.
**
**	Returns:
**		A pointer to the created node.
**
**	Side Effects:
**		Space is taken from Qbuf.
*/


QTREE *
tree(lptr, rptr, typ, len, value)
QTREE	*lptr;
QTREE	*rptr;
char	typ;
int	len;
int	value;
{
	register QTREE	*tptr;
	extern char	*need();
	register int	l;

	l = len;

	tptr = (QTREE *) need(Qbuf, l + QT_HDR_SIZ);
	tptr->left = lptr;
	tptr->right = rptr;
	tptr->sym.type = typ;
	tptr->sym.len = l;

	if (l > 0)
		tptr->sym.value.sym_data.i2type = value;
	return (tptr);
}
