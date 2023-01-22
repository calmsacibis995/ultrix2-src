#ifndef lint
static	char	*sccsid = "@(#)getequal.c	1.1	(ULTRIX)	1/8/85";
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
**	getequal - get the first tuple equal to the provided key
**	
**	GETEQUAL is used to do a keyed retrieval of a single
**	tuple in cases where the calling program knows the key to
**	be unique.  SETKEY must be called first to set all desired
**	domain values.  GETEQUAL will return the first tuple with
**	equality on all of the specified domains.
**	The tuple is returned in TUPLE.
**	
**	function values:
**	
**		<0 fatal error
**		 0  success
**		 1  no match
**
**	Trace Flags:
**		23.8-15
*/


getequal(d, key, tuple, tid)
register DESC	*d;
char		key[MAXTUP];
char		tuple[MAXTUP];
TID		*tid;
{
	TID		limtid;
	register int	i;

#	ifdef xATR1
	if (tTf(23, 8))
	{
		printf("getequal: %.14s,", d->reldum.relid);
		printdesc(d);
		printup(d, key);
	}
#	endif
	if (i = find(d, EXACTKEY, tid, &limtid, key))
		return (i);
	while ((i = get(d, tid, &limtid, tuple, TRUE)) == 0)
	{
		if (!kcompare(d, key, tuple))
		{
#			ifdef xATR2
			if (tTf(23, 9))
			{
				printf("getequal: ");
				dumptid(tid);
				printf("getequal: ");
				printup(d, tuple);
			}
#			endif
			return (0);
		}
	}
#	ifdef xATR2
	if (tTf(23, 10))
		printf("getequal: %d\n", i);
#	endif
	return (i);
}
