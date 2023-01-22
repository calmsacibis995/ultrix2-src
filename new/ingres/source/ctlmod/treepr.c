#ifndef lint
static	char	*sccsid = "@(#)treepr.c	1.1	(ULTRIX)	1/8/85";
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
# include	<tree.h>
# include	<aux.h>


/*
**  TREEPR -- print tree for debugging
**
**	This routine prints a tree for debugging.
**
**	Parameters:
**		tree -- root of tree to be printed
**
**	Returns:
**		none
**
**	Side Effects:
**		output to terminal
*/

treepr(tree)
QTREE	*tree;
{
	register QTREE	*t;
	register int	i;

	t = tree;

	printf("Querytree @ %x:\n", t);

	/* print range table */
	for (i = 0; i < MAXVAR + 1; i++)
	{
		if (Qt.qt_rangev[i].rngvdesc == NULL)
			continue;
		printf("range of %d is %.14s\n",
		    i, Qt.qt_rangev[i].rngvdesc->reldum.relid);
	}

	/* print query type */
	if (Qt.qt_qmode >= 0)
		printf("Qmode %d ", Qt.qt_qmode);

	/* print result relation if realistic */
	if (Qt.qt_resvar >= 0)
		printf("resvar %d ", Qt.qt_resvar);
	printf("\n");

	/* print tree */
	rcsvtrpr(t);

	/* print exciting final stuff */
	printf("\n");
}
/*
**  RCSVTRPR -- traverse and print tree
**
**	This function does the real stuff for treepr.  It recursively
**	traverses the tree in postfix order, printing each node.
**
**	Parameters:
**		tree -- the root of the tree to print.
**
**	Returns:
**		none
**
**	Side Effects:
**		none
**
**	Trace Flags:
**		none
*/

static
rcsvtrpr(tree)
QTREE	*tree;
{
	register QTREE	*t;

	t = tree;

	while (t != NULL)
	{
		nodepr(t);
		rcsvtrpr(t->left);
		t = t->right;
	}
}
/*
**  NODEPR -- print tree node for debugging
**
**	Parameters:
**		tree -- the node to print.
**
**	Returns:
**		none
**
**	Side Effects:
**		output to terminal
*/

nodepr(tree)
QTREE	*tree;
{
	register QTREE	*t;
	register int	ty;
	int		l;
	char		*cp;

	t = tree;
	ty = t->sym.type;
	l = t->sym.len & I1MASK;

	printf("%x: %x, %x/ ", t, t->left, t->right);
	xputchar(ty);
	printf("%d: ", l);

	switch (ty)
	{
	  case VAR:
		printf("%d.%d [", t->sym.value.sym_var.varno,
		    t->sym.value.sym_var.attno);
		xputchar(t->sym.value.sym_var.varfrmt);
		printf("%d]: %x", t->sym.value.sym_var.varfrml & I1MASK,
				  t->sym.value.sym_var.valptr);
		if (t->sym.value.sym_var.varno == -1)
		{
			printf("\n\tSub var: ");
			if (t->sym.value.sym_var.valptr != NULL)
				nodepr((QTREE *) t->sym.value.sym_var.valptr);
			else
				printf("ERROR: no value\n");
			return;
		}
		else
		{
			if (t->sym.value.sym_var.valptr != NULL)
			{
				printf(" = ");
				printatt(t->sym.value.sym_var.varfrmt,
					 t->sym.value.sym_var.varfrml,
					 t->sym.value.sym_var.valptr);
			}
		}
		break;

	  case RESDOM:
		printf("%d [%c%d]", t->sym.value.sym_resdom.resno,
		    t->sym.value.sym_resdom.resfrmt,
		    t->sym.value.sym_resdom.resfrml);
		break;

	  case AOP:
		printf("%d [%c%d] [%c%d]", t->sym.value.sym_op.opno,
		    t->sym.value.sym_op.opfrmt, t->sym.value.sym_op.opfrml,
		    t->sym.value.sym_op.agfrmt, t->sym.value.sym_op.agfrml);
		break;

	  case UOP:
	  case BOP:
	  case COP:
	  case INT:
	  case QMODE:
	  case RESULTVAR:
		switch (t->sym.len)
		{
		  case 1:
		  case 2:
			printf("%d", t->sym.value.sym_data.i2type);
			break;
		
		  case 4:
			printf("%ld", t->sym.value.sym_data.i4type);
			break;
		}
		break;

	  case FLOAT:
		printf("%.10f", t->sym.value.sym_data.f4type);
		break;

	  case CHAR:
		cp = t->sym.value.sym_data.c0type;
		while (l--)
			xputchar(*cp++);
		break;

	  case AND:
	  case ROOT:
	  case AGHEAD:
		printf("[%d/%d] [%o/%o]",
		    t->sym.value.sym_root.tvarc, t->sym.value.sym_root.lvarc,
		    t->sym.value.sym_root.lvarm, t->sym.value.sym_root.rvarm);
		if (ty != AND)
			printf(" (%d)", t->sym.value.sym_root.rootuser);
		break;

	  case TREE:
	  case OR:
	  case QLEND:
	  case BYHEAD:
		break;

	  case RESULTID:
	  case SOURCEID:
		printf("%.14s", t->sym.value.sym_data.c0type);
		break;

	  default:
		syserr("nodepr: ty %d", ty);
	}
	printf("/\n");
}
