#ifndef lint
static	char	*sccsid = "@(#)inpcloser.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>


/*
**	inpcloser - close an input relation
**
**	The relation must have been opened by openr with 
**		mode 0 (read only)
**
**	return values:
**		<0 fatal error
**		 0 success
**		 1 relation was not open
**		 2 relation was opened in write mode
**
**	Trace Flags:
**		21.10-11
*/

inpcloser(d)
register DESC	*d;
{
	register int	i;

#	ifdef xATR1
	if (tTf(21, 10))
		printf("inpcloser: %.14s\n", d->reldum.relid);
#	endif
	if (abs(d->relopn) != (d->relfp + 1) * 5)
		/* relation not open */
		return (1);

	if (d->relopn < 0)
		return (2);	/* relation open in write mode */

	i = flush_rel(d, TRUE);	/* flush and reset all pages */

	if (close(d->relfp))
		i = acc_err(AMCLOSE_ERR);
	d->relopn = 0;
	return (i);
}
