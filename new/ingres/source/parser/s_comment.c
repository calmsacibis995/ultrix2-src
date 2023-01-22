#ifndef lint
static	char	*sccsid = "@(#)s_comment.c	1.1	(ULTRIX)	1/8/85";
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


# include <ingres.h>
# include "scanner.h"


/*
** COMMENT
** scans comments (as delimited by the tokens 'Tokens.bgncmnt'
** and 'Tokens.endcmnt') and removes them from the query text.
*/
comment()
{
	register int		i, l;
	register struct optab	*op;
	register	char	*sp;
	char			buf[3];

	/* find the end_of_comment operator */
	for (op = Optab; op->term; op++)
		if (op->token == Tokens.endcmnt)
			break;
	if (!op->term)
		syserr("no end_of_comment token");

	/* scan for the end of the comment */
	l = length(op->term);
	for (i = 0,sp = buf; i < l; sp++, i++)		/* set up window on input */
		if ((*sp = get_scan(NORMAL)) <= 0)
			/* non-terminated comment */
			par_error(COMMTERM, FATAL, 0);		/* must end parsing */
	while (!bequal(buf, op->term, l))
	{
		/* move window on input */
		for (sp = buf,i = 0; i < l-1; i++,sp++)
			*sp = *(sp+1);
		if (( *sp = get_scan(NORMAL)) <= 0)
			/* non terminated comment */
			par_error(COMMTERM, FATAL, 0);		/* must end parsing */
	}
	return (0);
}
