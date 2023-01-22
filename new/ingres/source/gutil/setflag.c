#ifndef lint
static	char	*sccsid = "@(#)setflag.c	1.1	(ULTRIX)	1/8/85";
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
**  SET FLAG
**
**	This routine sets flags from argv.  You give arguments
**	of the argv, the flag to be detected, and the default
**	for that flag (if it is not supplied).  The return value
**	is the flag value.   For example:
**		setflag(argv, 'x', 2);
**	returns zero if the "-x" flag is stated, and one if the
**	"+x" flag is stated.  It returns 2 if the flag is not
**	stated at all.
*/

# define	NULL	0

setflag(argv, flagch, def)
char	**argv;
char	flagch;
int	def;
{
	register char	**p;
	register char	*q;
	register int	rtval;

	rtval = -1;
	for (p = &argv[1]; *p != NULL; p++)
	{
		q = *p;
		if (q[1] != flagch)
			continue;
		if (*q != '-' && *q != '+')
			continue;
		rtval = (q[0] == '+');
	}
	if (rtval < 0)
		rtval = def;
	return (rtval);
}
