#ifndef lint
static	char	*sccsid = "@(#)xdot.c	1.1	(ULTRIX)	1/8/85";
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
# include	"parser.h"


extern QTREE		*tree();
extern QTREE		*addresdom();

/*
** XDOT
**	add to attribute stash any missing attributes in the
**	source relation and then build tree with all attribs
**	in the 'attid' order.  This algorithm assumes that
**	the function 'attadd' insert attributes into the list
**	in 'attid' order from 1 -> N.
*/
QTREE *
xdot(slot)
int	slot;
{
	PARRNG				*rptr;
	struct attribute		tuple;
	register struct attribute	*ktuple;
	struct attribute		ktup;
	TID				tid;
	TID				limtid;
	QTREE		*tempt;
	register QTREE	*vnode;
	int				ik;
	register struct atstash		*aptr;

	extern PARRNG			Parrng[];
	extern char			*Trname;
	extern DESC			Attdes;

	rptr = &Parrng[slot];

#	ifdef	xPTR2
	tTfp(35, 0, "ALL being processed for %12s\n",
	    rptr->vardesc.relvname);
#	endif

	if (rptr->vardesc.reldum.relatts <= 0)
		syserr("xdot: rptr->vardesc.reldum.relatts %d.\n", rptr->vardesc.reldum.relatts);
	/* if attstash is missing any attribs then fill in list */
	if (rptr->vardesc.reldum.relatts != attcount(slot))
	{
		/* get all entries in attrib relation */
		clearkeys(&Attdes);
		ktuple = &ktup;
		setkey(&Attdes, ktuple, rptr->vardesc.reldum.relid, ATTRELID);
		setkey(&Attdes, ktuple, rptr->vardesc.reldum.relowner, ATTOWNER);
		if (ik = find(&Attdes, EXACTKEY, &tid, &limtid, ktuple))
			syserr("bad find in xdot '%d'", ik);
		while (!get(&Attdes, &tid, &limtid, &tuple, 1))
			if (!kcompare(&Attdes, &tuple, ktuple))
				/* add any that are not in the stash */
				if (!attfind(slot, tuple.attname))
					attadd(slot, &tuple);
	}

	/* build tree for ALL */
	tempt = NULL;
	aptr = rptr->attlist;
	while (aptr != 0)
	{
		vnode = tree(NULL, NULL, VAR, sizeof(struct varnode), slot, aptr);
		Trname = aptr->atbname;
		tempt = addresdom(tempt, vnode);
		aptr = aptr->atbnext;
	}

#	ifdef	xPTR3
	tTfp(35, 0, "end of xdot %12s\n", rptr->vardesc.relvname);
#	endif

	return(tempt);
}
