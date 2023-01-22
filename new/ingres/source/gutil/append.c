#ifndef lint
static	char	*sccsid = "@(#)append.c	1.1	(ULTRIX)	1/8/85";
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
**  APPEND -- block concatenate
**
**	block `b1' of length `l1' is concatenated to block
**	`b2' of length `l2', giving `b3'.
**
**	Returns the address of the next byte available after
**	the end of `b3'.
*/

char *
append(b1, l1, b2, l2, b3)
int	l1, l2;
char	*b1, *b2, *b3;
{
	register char	*p, *q;
	register int	n;

	p = b3;
	n = l1;
	q = b1;
	while (n-- > 0)
		*p++ = *q++;
	n = l2;
	q = b2;
	while (n-- > 0)
		*p++ = *q++;
	return (p);
}
