#ifndef lint
static	char	*sccsid = "@(#)get_scan.c	1.1	(ULTRIX)	1/8/85";
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
# include	"scanner.h"


/*
** GET_SCAN -- gets characters from monitor
**
**	Parameters:
**		mode --
**	   	    modes are:
**			NORMAL = read normally
**			PRIME = prime the pipe
**			SYNC = sync (or flush) the pipe
**
**	Returns:
**		character or '\0' on eof
**
**	Trace Flags:
**		Getscan ~~ 54.0
*/

get_scan(mode)
int	mode;
{
	extern int		yyline;
	register int		ctr;
	char			c;

	extern int		Pctr;		/* vble for backup stack in scanner */
	extern char		Pchar[2];
# ifdef	xPTR3
	tTfp(54, 0, "get_scan: mode %d ", mode);
# endif

	switch (mode)
	{
	    case NORMAL:
		if (Pctr)
		{
			c = Pchar[--Pctr];
			ctr = 1;
		}
		else
			ctr = readmon(&c, 1);
		if (c == '\n')
			yyline++;
		c = ((Lcase && c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c);
		break;

	    case PRIME:
		Pctr = 0;
		ctr = 0;
		break;

	    case SYNC:				/* flush pipe */
		while (readmon(&c, 1) > 0);
		ctr = 0;
		break;

	    default:
		syserr("bad arg '%d' in get_scan", mode);
	}

# ifdef	xPTR3
	tTfp(54, 1, " ctr %d: '%c' (0%o).\n", ctr & 0377, c, c);
# endif

	return (ctr ? c : 0);
}
