#ifndef lint
static	char	*sccsid = "@(#)pmove.c	1.1	(ULTRIX)	1/8/85";
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



/*
**  Packed Move
**
**	Moves string `s' to storage area `b' of length `l' bytes.  If
**	`s' is too long, it is truncated, otherwise it is padded to
**	length `l' with character `c'.  `B' after the transfer is
**	returned.
*/

char *
pmove(s1, b1, l1, c)
char	*s1;
char	*b1;
int	l1;
char	c;
{
	register char	*s;
	register char	*b;
	register int	l;

	s = s1;
	b = b1;
	l = l1;

	/* move up to `l' bytes */
	while (*s && l > 0)
	{
		*b++ = *s++;
		l -= 1;
	}

	/* if we still have some `l', pad */
	while (l-- > 0)
	{
		*b++ = c;
	}

	return (b);
}
