#ifndef lint
static	char	*sccsid = "@(#)issue.c	1.1	(ULTRIX)	1/8/85";
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
# include	<symbol.h>
# include	<tree.h>
# include	<pv.h>
# include	"qrymod.h"


/*
**  ISSUE -- issue query to rest of system
**
**	This function issues a query to the rest of the INGRES system.
**	The sync from below is read, but not passed up.
**
**	Parameters:
**		tree -- pointer to tree to issue.
**
**	Returns:
**		none.
**
**	Side Effects:
**		A query is executed.
**
**	Trace Flags:
**		71
*/

issue(state, tree)
int	state;
QTREE	*tree;
{
#	ifdef xQTR2
	if (tTf(71, 0))
		printf("issue:\n");
#	endif

	initp();
	setp(PV_QTREE, tree, 0);
	call(state, NULL);
}
/*
**  ISSUEINVERT -- issue a query, but invert the qualification
**
**	This routine is similar to 'issue', except that it issues
**	a query with the qualification inverted.  The inversion
**	(and subsequent tree normalization) is done on a duplicate
**	of the tree.
**
**	Parameters:
**		root -- the root of the tree to issue.
**
**	Returns:
**		none.
**
**	Side Effects:
**		'root' is issued.
**
**	Trace Flags:
**		none
*/

issueinvert(root)
QTREE	*root;
{
	register QTREE	*t;
	register QTREE	*uop;
	extern QTREE	*treedup();
	extern QTREE	*trimqlend(), *norml();

	/* make duplicate of tree */
	t = treedup(root);

	/* prepend NOT node to qualification */
	uop = (QTREE *) need(Qbuf, QT_HDR_SIZ + sizeof(short));
	uop->left = NULL;
	uop->right = t->right;
	uop->sym.type = UOP;
	uop->sym.len = sizeof(short);
	uop->sym.value.sym_op.opno = opNOT;
	t->right = uop;

	/* normalize and issue */
	t->right = norml(trimqlend(t->right));
	issue(mdQRY, t);
}
