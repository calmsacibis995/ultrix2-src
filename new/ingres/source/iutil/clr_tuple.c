#ifndef lint
static	char	*sccsid = "@(#)clr_tuple.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>



/*
**	Clr_tuple initializes all character domains
**	to blank and all numeric domains to zero.
*/

clr_tuple(desc, tuple)
struct descriptor	*desc;
char			*tuple;
{
	register struct descriptor	*d;
	register char			*tup;
	register int			i;
	int				j, pad;

	d = desc;

	for (i = 1; i <= d->reldum.relatts; i++)
	{
		if (d->relfrmt[i] == CHAR)
			pad = ' ';
		else
			pad = 0;

		tup = &tuple[d->reloff[i]];
		j = d->relfrml[i] & I1MASK;

		while (j--)
			*tup++ = pad;
	}
}
