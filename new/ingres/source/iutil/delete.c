#ifndef lint
static	char	*sccsid = "@(#)delete.c	1.1	(ULTRIX)	1/8/85";
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
**	Delete - delete the tuple specified by tid
**
**	Delete removes the tuple specified by tid
**	and reclaims the tuple space.
**
**	returns:
**		<0  fatal error
**		0   success
**		2   tuple specified by tid aleady deleted
*/

delete(dx, tidx)
DESC	*dx;
TID	*tidx;
{
	register DESC	*d;
	register TID	*tid;
	register int	i;

	d = dx;
	tid = tidx;

#	ifdef xATR1
	if (tTf(24, 8))
	{
		printf("delete: %.14s,", d->reldum.relid);
		dumptid(tid);
	}
#	endif

	if (i = get_page(d, tid))
		return (i);

	if (i = invalid(tid))
		return (i);

	i = tup_len(tid);

	del_tuple(tid, i);
	d->reladds--;

	return (0);
}
