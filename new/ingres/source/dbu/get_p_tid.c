#ifndef lint
static	char	*sccsid = "@(#)get_p_tid.c	1.1	(ULTRIX)	1/8/85";
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
# include	<catalog.h>


/*
**  GET_P_TID -- Get the primary tid for a relation for locking
**
**	Finds the correct tid for locking the relation. If the
**	relation is a primary relation, then the tid of the
**	relation is returned.
**
**	If the relation is a secondary index then the tid of the
**	primary relation is returned.
**
**	Parameters:
**		des - an open descriptor for the relation.
**		tidp - a pointer to a place to store the tid.
**
**	Returns:
**		none
**
**	Side Effects:
**		alters the value stored in "tidp",
**		may cause access to the indexes relation
**
**	Called By:
**		modify
*/



get_p_tid(d, tp)
register DESC	*d;
register TID	*tp;
{
	register int	i;
	struct index	indkey, itup;
	DESC		ides;
	extern DESC	Inddes;

	if (d->reldum.relindxd < 0)
	{
		/* this is a secondary index. lock the primary rel */
		opencatalog("indexes", 0);
		setkey(&Inddes, &indkey, d->reldum.relowner, IOWNERP);
		setkey(&Inddes, &indkey, d->reldum.relid, IRELIDI);
		if (getequal(&Inddes, &indkey, &itup, tp))
			syserr("No prim for %.14s", d->reldum.relid);

		if (i = openr(&ides, -1, itup.irelidp))
			syserr("openr prim %d,%.14s", i, itup.irelidp);

		bmove(&ides.reltid, tp, sizeof (*tp));
	}
	else
		bmove(&d->reltid, tp, sizeof (*tp));
}
