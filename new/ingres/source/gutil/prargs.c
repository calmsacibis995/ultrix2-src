#ifndef lint
static	char	*sccsid = "@(#)prargs.c	1.1	(ULTRIX)	1/8/85";
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
**  PRARGS -- print argument list
**
**	Takes an argument list such as expected by any main()
**	or the DBU routines and prints them on the standard
**	output for debugging purposes.
**
**	Parameters:
**		pc -- parameter count.
**		pv -- parameter vector (just like to main()).
**
**	Returns:
**		nothing
**
**	Side Effects:
**		output to stdout only.
*/

prargs(pc, pv)
int	pc;
char	**pv;
{
	register char	**p;
	register char	c;
	int		n;
	register char	*q;

	n = pc;
	printf("#args=%d:\n", n);
	for (p = pv; n-- > 0; p++)
	{
		q = *p;
		while ((c = *q++) != 0)
			xputchar(c);
		putchar('\n');
	}
}
