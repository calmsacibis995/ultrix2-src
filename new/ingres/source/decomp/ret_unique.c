#ifndef lint
static	char	*sccsid = "@(#)ret_unique.c	1.1	(ULTRIX)	1/8/85";
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
**	create a result relation for a ret_unique
*/

mk_unique(root)
QTREE	*root;
{
	register int	i, domcnt;
	register QTREE	*r;

	r = root;

	/* verify that target list is within range */
	domcnt = r->left->sym.type != TREE ? r->left->sym.value.sym_resdom.resno : 0;
	if (findwid(r) > MAXTUP || domcnt > MAXDOM)
		derror(4620);
	i = MAXRANGE - 1;
	De.de_rangev[i].relnum = mak_t_rel(r, "u", -1);
	De.de_resultvar = i;

	/* don't count retrieve into portion as a user query */
	r->sym.value.sym_root.rootuser = 0;
}
/*
**	Retrieve all domains of the variable "var".
**	This routine is used for ret_unique to retrieve
**	the result relation. First duplicates are removed
**	then the original tree is converted to be a
**	retrieve of all domains of "var", and then
**	ovqp is called to retrieve the relation.
*/

pr_unique(root1, var1)
QTREE	*root1;
int			var1;
{
	register QTREE	*root, *r;
	register int	var;
	extern QTREE	*makavar();

	root = root1;
	var = var1;

	/* remove duplicates from the unopened relation */
	removedups(var);

	/* remove the qual from the tree */
	root->right = De.de_qle;

	/* make all resdoms refer to the result relation */
	for (r = root->left; r->sym.type != TREE; r = r->left)
		r->right = makavar(r, var, r->sym.value.sym_resdom.resno);

	/* count as a user query */
	root->sym.value.sym_root.rootuser = TRUE;

	/* run the retrieve */
	De.de_sourcevar = var;
	De.de_newq = De.de_newr = TRUE;
	call_ovqp(root, mdRETR, NORESULT);
}
