#ifndef lint
static	char	*sccsid = "@(#)oatoi.c	1.1	(ULTRIX)	1/8/85";
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
**  OCTAL ASCII TO INTEGER CONVERSION
**
**	The ascii string 'a' which represents an octal number
**	is converted to binary and returned.  Conversion stops at any
**	non-octal digit. The integer reflects the natural word length of the
**	machine.
**
**	Note that the number may not have a sign, and may not have
**	leading blanks.
**
**	(Intended for converting the status codes in users(FILE))
*/

oatoi(a)
char	*a;
{
	register int	r;
	register char	*p;
	register char	c;

	r = 0;
	p = a;

	while ((c = *p++) >= '0' && c <= '7')
		r = (r << 3) | (c &= 7);

	return (r);
}
