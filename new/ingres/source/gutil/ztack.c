#ifndef lint
static	char	*sccsid = "@(#)ztack.c	1.1	(ULTRIX)	1/8/85";
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
**  LOCAL STRING CONCATENATE
**	Strings `a' and `b' are concatenated and left in an
**	internal buffer.  A pointer to that buffer is returned.
**
**	Ztack can be called recursively as:
**		ztack(ztack(ztack(w, x), y), z);
*/

char *
ztack(a, b)
register char	*a, *b;
{
	register char	*c;
	static char	buf[101];
	
	c = buf;
	
	while (*a)
		*c++ = *a++;
	while (*b)
		*c++ = *b++;
	*c = '\0';
	if (buf[100] != 0)
		syserr("ztack overflow: %s", buf);
	return (buf);
}
