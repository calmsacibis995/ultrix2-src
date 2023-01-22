#ifndef lint
static	char	*sccsid = "@(#)ageval.c	1.1	(ULTRIX)	1/8/85";
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
# include	<pv.h>
# include	"globs.h"


/*
**	AGEVAL -- evaluate simple aggregate.
**
**	Ageval is passed the tree of a simple aggregate,
**	and an array of space to store the results. The
**	amount of space actually allocated is stored in
**	(*result)->sym.len
**
**	If the aggregate is unique (eg. countu, sumu, avgu)
**	or if the aggregate is multi-variable, special
**	processing is done. A temporary relation is formed
**	with a result domain for each single agg in the tree.
**	Decomp is called to retrieve
**	the values to be aggregated into that relation.
**
**	If the aggregate is unique, then duplicates are
**	removed from the temporary relation.
**
**	Next the aggregate is run on either the original relation
**	or on the temporary relation.
**
**	Finally the result is read from OVQP and if a
**	temporary relation was used, it is destroyed.
**
**	Trace Flags:
**		41
*/


QTREE *
ageval(tree, result)
QTREE	*tree;		/* root of aggregate */
QTREE	*result[];	/* space for results */
{
	register QTREE	*aghead, *resdom, *aop;
	QTREE		*newtree;
	QTREE		*lnodv[MAXDOM + 2];
	char		agbuf[AGBUFSIZ];
	int		temp_relnum, i;
	extern int	derror();
	extern QTREE	*makroot(), *makavar(), *makresdom();

#	ifdef xDTR1
	if (tTf(41, 2))
	{
		printf("entered ageval\n");
		treepr(tree);
	}
#	endif

	aghead = tree;
	aop = aghead->left;
	temp_relnum = NORESULT;

	/* if PRIME or multi-var, form aggregate domain in temp relation */
	if (prime(aop) || aghead->sym.value.sym_root.tvarc > 1)
	{
		initbuf(agbuf, AGBUFSIZ, AGBUFFULL, derror);

		lnodv[lnode(aop, lnodv, 0)] = 0;

		/* create new tree for retrieve and give it the qualification */
		newtree = makroot(agbuf);
		newtree->right = aghead->right;
		aghead->right = De.de_qle;

		/* put a resdom on new tree for each aop in orig tree */
		/* make each aop in orig tree reference new relation */
		for (i = 0; aop = lnodv[i]; )
		{

			/* create resdom for new tree */
			resdom = makresdom(agbuf, aop);
			resdom->sym.value.sym_resdom.resno = ++i;
			resdom->right = aop->right;

			/* connect it to newtree */
			resdom->left = newtree->left;
			newtree->left = resdom;

			/* make orig aop reference new relation */
			aop->right = makavar(resdom, FREEVAR, i);
		}

		/* make result relation */
		temp_relnum = mak_t_rel(newtree, "a", -1);

		/* prepare for query */
		mapvar(newtree, 0);
		decomp(newtree, mdRETR, temp_relnum);
		De.de_rangev[FREEVAR].relnum = temp_relnum;
		De.de_sourcevar = FREEVAR;

		/* if prime, remove dups */
		if (prime(aghead->left))
		{
			/* modify to heapsort */
			removedups(FREEVAR);
		}

	}

	De.de_newq = 1;
	De.de_newr = TRUE;

	call_ovqp(aghead, mdRETR, NORESULT);	/* call ovqp with no result relation */
	De.de_newq = 0;

	/* pick up results */
	readagg_result(result);

	/* if temp relation was created, destroy it */
	if (temp_relnum != NORESULT)
		dstr_rel(temp_relnum);

}
/*
**	Determine if an aggregate contains any
**	prime aggregates. Note that there might
**	be more than one aggregate.
*/

prime(aop)
QTREE	*aop;
{
	register QTREE	*a;

	a = aop;
	do
	{
		switch (a->sym.value.sym_op.opno)
		{
		  case opCOUNTU:
		  case opSUMU:
		  case opAVGU:
			return (TRUE);
		}
	} while (a = a->left);
	return (FALSE);
}
/*
**	Remove dups from an unopened relation
**	by calling heapsort
*/

removedups(var)
int	var;
{
	register char	*p;
	char		*rangename();

	closer1(var);	/* guarantee that relation has been closed */
	initp();
	p = rangename(var);	/* get name of relation */
#	ifdef xDTR1
	if (tTf(41, 1))
	{
		printf("removing dups from %s\n", p);
	}
#	endif
	setp(PV_STR, p);
	setp(PV_STR,"heapsort");
	setp(PV_STR,"num");
	call_dbu(mdMODIFY, FALSE);
}
