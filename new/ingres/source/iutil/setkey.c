#ifndef lint
static	char	*sccsid = "@(#)setkey.c	1.1	(ULTRIX)	1/8/85";
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


/*
**	Clearkeys - reset all key indicators in descriptor
**
**	Clearkeys is used to clear the key supplied
**	flags before calls to setkey
*/


clearkeys(d)
register DESC	*d;
{
	register int	i;

	for (i = 0; i <= d->reldum.relatts; i++)
		d->relgiven[i] = 0;
	return (0);
}
/*
**  SETKEY - indicate a partial key for find
**
**	Setkey is used to set a key value into place
**	in a key. The key can be as large as the entire
**	tuple. Setkey moves the key value into the
**	proper place in the key and marks the value as
**	supplied
**
**	If the value is a null pointer, then the key is
**	cleared.
**
**	Clearkeys should be called once before the
**	first call to setkey.
*/

setkey(d, key, value, dom)
register DESC	*d;
char		*key;
char		*value;
register int	dom;
{
	register int	len;
	char		*cp;

#	ifdef xATR1
	if (tTf(22, 8))
		printf("setkey: %.14s, %d\n", d->reldum.relid, dom);
#	endif

	/* check validity of domain number */
	if (dom < 1 || dom > d->reldum.relatts)
		syserr("setkey:rel=%.12s,dom=%d", d->reldum.relid, dom);

	/* if value is null, clear key */
	if (value == 0)
	{
		d->relgiven[dom] = 0;
		return;
	}

	/* mark as given */
	d->relgiven[dom] = 1;

	len = d->relfrml[dom] & I1MASK;
	cp = &key[d->reloff[dom]];

	if (d->relfrmt[dom] == CHAR)
	{
		pmove(value, cp, len, ' ');
	}
	else
	{
		bmove(value, cp, len);
	}
}
