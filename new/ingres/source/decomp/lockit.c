#ifndef lint
static	char	*sccsid = "@(#)lockit.c	1.1	(ULTRIX)	1/8/85";
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
# include	"globs.h"
# include	<lock.h>


/*
**  LOCKIT -- sets relation locks for integrity locking
**
**	Parameters:
**		root- the root of a query tree;
**		resvar- index of variable to be updated.
**
*/
lockit(root, resvar)
QTREE	*root;
int	resvar;
{
	register QTREE	*r;
	register int	i, j;
	long		vlist[MAXRANGE];
	int		bmap, cv;
	char		mode;
	int		skvr;
	int		redo;
	DESC		*d, *openr1();
	long		restid;
	int		k;

	r = root;
	bmap = r->sym.value.sym_root.lvarm | r->sym.value.sym_root.rvarm;
	if (resvar >= 0)
		bmap |= 01 << resvar;
	else
		restid = -1;
	i = 0;
	/* put relids of relations to be locked into vlist
	   check for and remove duplicates */
	for (j = 0; j < MAXRANGE; j++)
		if (bmap & (01 << j))
		{
			d = openr1(j);
			if (j == resvar)
				restid = d->reltid.ltid;
			for (k = 0; k < i; k++)
				if (vlist[k] == d->reltid.ltid)
					break;
			if (k == i)
				vlist[i++] = d->reltid.ltid;
		}
	cv = i;
/*
 *	set the locks: set the first lock with the sleep option
 *			set other locks checking for failure;
 *			if failure, release all locks, sleep on blocking
 *			lock.
 */
	skvr = -1;
	do
	{
		/* skvr is the index of the relation already locked
		   try to lock the remaining relations */
		redo = FALSE;
		for (i = 0; i < cv; i++)
			if (i != skvr)
			{
				if (restid == vlist[i])
					mode = M_EXCL;
				else
					mode = M_SHARE;
				if (setrll(A_RTN, vlist[i], mode) < 0)
					/* a lock request failed */
				{
					unlall();	/* release all locks */
					setrll(A_SLP, vlist[i], mode);
							/* wait on problem lock*/
					skvr = i;
					redo = TRUE;
					break;	/* reset the other locks */
				}
			}
	}
	while (redo);
}
