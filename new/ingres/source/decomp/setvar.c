#ifndef lint
static	char	*sccsid = "@(#)setvar.c	1.1	(ULTRIX)	1/8/85";
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
** SETVAR -- Routines to convert a VAR to a constant and back to a VAR
**
**	This file contains the routines for tuple substitution.
**
**	Setvar -- Make a VAR node reference its position in the tuple.
**
**	Clrvar -- Make the VAR node refer back to a VAR
*/
/*
**  SETVAR
**
**	Var's are changed to reference their values in a tuple.
**	ROOT and AND nodes are changed to update the variable maps.
**
**	Trace Flags:
**		53
*/

setvar(tree, var, intid, tuple)
register QTREE 	*tree;
int		var;
TID		*intid;
char		*tuple;
{
	register int	mask, nvc;
	DESC		*readopen();
	extern QTREE	*ckvar();

	if (tree == NULL) 
		return;
	switch (tree->sym.type)
	{
	  case VAR:
		if ((tree=ckvar(tree))->sym.value.sym_var.varno == var)
		{
#			ifdef xDTR1
			if (tTf(53, 0))
			{
				printf("setvar:%d;tree:", var);
				nodepr(tree);
			}
#			endif
			if (tree->sym.value.sym_var.attno)
				tree->sym.value.sym_var.valptr =
				    (ANYTYPE *) (tuple + readopen(var)->reloff[tree->sym.value.sym_var.attno]);
			else
				tree->sym.value.sym_var.valptr = (ANYTYPE *) intid;
		}
		return;

	  case ROOT:
	  case AND:
		mask = 01 << var;
		nvc = tree->sym.value.sym_root.tvarc;
		if (tree->sym.value.sym_root.lvarm & mask)
		{
			setvar(tree->left, var, intid, tuple);
			tree->sym.value.sym_root.lvarm &=  ~mask;
			--tree->sym.value.sym_root.lvarc;
			nvc = tree->sym.value.sym_root.tvarc - 1;
		}
		if (tree->sym.value.sym_root.rvarm & mask)
		{
			setvar(tree->right, var, intid, tuple);
			tree->sym.value.sym_root.rvarm &=  ~mask;
			nvc = tree->sym.value.sym_root.tvarc - 1;
		}
		tree->sym.value.sym_root.tvarc = nvc;
		return;

	  default:
		setvar(tree->left, var, intid, tuple);
		setvar(tree->right, var, intid, tuple);
		return;
	}
}
/*
**	Clearvar is the opposite of setvar. For
**	each occurence of var1 in the tree, clear
**	the valptr.
*/

clearvar(tree, var)
register QTREE	*tree;
register int	var;
{
	extern QTREE	*ckvar();

	if (tree == NULL)
		return;

	if (tree->sym.type == VAR)
	{
		if ((tree = ckvar(tree))->sym.value.sym_var.varno == var)
			tree->sym.value.sym_var.valptr = 0;
		return;
	}
	clearvar(tree->left, var);
	clearvar(tree->right, var);
}
