#ifndef lint
static	char	*sccsid = "@(#)purgetup.c	1.1	(ULTRIX)	1/8/85";
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
**	Remove tuples from the specified system relation.
**
**	'Desa' is a descriptor for a system relation and
**	key[12] and dom[12] are the keys and domain numbers
**	to match on for the delete.
**	All the tuples in 'desa' with key1 and key2
**	are deleted from the relation.
*/

purgetup(d, key1, dom1, key2, dom2)
register DESC	*d;
char		*key1;
int		dom1;
char		*key2;
int		dom2;
{
	TID		tid, limtid;
	register int	i;
	char		tupkey[MAXTUP], tuple[MAXTUP];

	setkey(d, tupkey, key1, dom1);
	setkey(d, tupkey, key2, dom2);
	if (i = find(d, EXACTKEY, &tid, &limtid, tupkey))
		syserr("purgetup:find:%d", i);
	while ((i = get(d, &tid, &limtid, tuple, TRUE)) == 0)
	{
		if (kcompare(d, tuple, tupkey) == 0)
			if (i = delete(d, &tid))
				syserr("attflush: delete %d", i);
	}

	if (i < 0)
		syserr("purgetup:get %.14s:%d", d->reldum.relid, i);
}
