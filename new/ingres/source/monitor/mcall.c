#ifndef lint
static	char	*sccsid = "@(#)mcall.c	1.1	(ULTRIX)	1/8/85";
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




/*
**  MCALL -- call a macro
**
**	This takes care of springing a macro and processing it for
**	any side effects.  Replacement text is saved away in a static
**	buffer and returned.
**
**	Parameters:
**		mac -- the macro to spring.
**
**	Returns:
**		replacement text.
**
**	Side Effects:
**		Any side effects of the macro.
**
**	Trace Flags:
**		51
*/

char *
mcall(mac)
char	*mac;
{
	register char	c;
	register char	*m;
	register char	*p;
	static char	buf[100];
	extern char	macsget();

	m = mac;

#	ifdef xMTR2
	tTfp(51, -1, "mcall('%s')\n", m);
#	endif

	/* set up to process the macro */
	macinit(macsget, &mac, FALSE);

	/* process it -- throw away result */
	for (p = buf; (c = macgetch()) > 0; )
	{
#		ifdef xMTR2
		if (tTf(51, 1))
			putchar(c);
#		endif
		if (p < &buf[sizeof buf])
			*p++ = c;
	}

	*p = 0;

	return (buf);
}
