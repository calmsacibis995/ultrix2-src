#ifndef lint
static	char	*sccsid = "@(#)getufield.c	1.1	(ULTRIX)	1/8/85";
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
**  GETUFIELD -- extract field from users file
**
**	A buffer returned as the result of a getuser() (or getpw())
**	call is scanned for the indicated parameter, numbered from
**	zero.  A pointer to the parameter is returned.
*/

char *
getufield(buf, num)
char	*buf;
int	num;
{
	register char	c;
	register int	i;
	register char	*p;
	char		*r;

	p = buf;

	/* skip other fields */
	for (i = num; i > 0; i--)
	{
		while ((c = *p++) != 0)
			if (c == ':')
				break;
	}

	/* save result pointer */
	r = p;

	/* null-terminate this field */
	while ((c = *p++) != 0)
		if (c == ':')
			break;

	*--p = 0;

	return (r);
}
