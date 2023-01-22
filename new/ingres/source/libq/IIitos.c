#ifndef lint
static	char	*sccsid = "@(#)IIitos.c	1.1	(ULTRIX)	1/8/85";
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
**  INTEGER OUTPUT CONVERSION
**
**	The integer `i' is converted to ascii and put
**	into the static buffer `buf'.  The address of the starting
**	point in `buf' is
**	returned.
**
**	Number is converted from least significant forwards.
*/

char *IIitos(i1)
int	i1;
{
	register char	*a;
	register int	i;
	static char	buf[25];

	i = i1;
	if (i < 0)
		i = -i;

	a = &buf[sizeof buf - 1];
	*a-- = '\0';
	do
	{
		*a-- = i % 10 + '0';
		i /= 10;
	} while (i);
	if (i1 < 0)
		*a-- = '-';

	a++;
	return (a);
}
