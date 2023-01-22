#ifndef lint
static	char	*sccsid = "@(#)IIsequal.c	1.1	(ULTRIX)	1/8/85";
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
**  IISEQUAL -- String equality comparison
**
**	Parameters:
**		s1, s2 -- strings to be tested for absolute equality
**
**	Returns:
**		1 -- if =
**		0 otherwise
**
**	Side Effects:
**		none
**
**	Called By:
**		IIgetpath() [IIingres.c]
*/

IIsequal(s1, s2)
char	*s1, *s2;
{
	register char	*r1, *r2;


	r1 = s1;
	r2 = s2;
	while (*r1 || *r2)
		if (*r1++ != *r2++)
			return (0);
	return (1);
}
