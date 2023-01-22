#ifndef lint
static	char	*sccsid = "@(#)insert.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>


/*
**	INSERT - add a new tuple to a relation
**
**	Insert puts a given tuple into a relation in
**	the "correct" position.
**
**	If insert is called with checkdups == TRUE then
**	the tuple will not be inserted if it is a duplicate
**	of some already existing tuple. If the relation is a
**	heap then checkdups is made false.
**
**	Tid will be set to the tuple id where the
**	tuple is placed.
**
**	returns:
**		<0  fatal eror
**		0   success
**		1   tuple was a duplicate
*/

insert(d, tid, tuple, checkdups)
register DESC	*d;
register TID	*tid;
char		*tuple;
bool		checkdups;
{
	register int	i;
	int		need;

#	ifdef xATR1
	if (tTf(24, 0))
	{
		printf("insert:%.14s,", d->reldum.relid);
		dumptid(tid);
		printup(d, tuple);
	}
#	endif
	/* determine how much space is needed for tuple */
	need = canonical(d, tuple);

	/* find the "best" page to place tuple */
	if (i = findbest(d, tid, tuple, need, checkdups))
		return (i);

	/* put tuple in position "tid" */
	put_tuple(tid, Acctuple, need);

	d->reladds++;

	return (0);
}
