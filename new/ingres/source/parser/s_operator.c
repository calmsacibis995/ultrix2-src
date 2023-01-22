#ifndef lint
static	char	*sccsid = "@(#)s_operator.c	1.1	(ULTRIX)	1/8/85";
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
** OPERATOR
** an operator is defined as any 1-3 character sequence of
** non-alphanumerics.  It looks up each operator in 'Optab'
** and returns the appropriate token.
*/
operator(chr)
char	chr;
{
	extern int		yylval;
	extern char		Cmap[];
	register struct optab	*op;
	register int		save;
	char			buf[4];

	/* get lookahead characer */
	save = Lcase;
	Lcase = 0;
	buf[0] = chr;
	buf[1] = get_scan(NORMAL);
	buf[2] = get_scan(NORMAL);
	buf[3] = '\0';

	/* is it a floating fraction without leading zero ? */
	if (buf[0] == '.' && Cmap[buf[1]] == NUMBR)
	{
		Lcase = save;
		backup(buf[2]);
		backup(buf[1]);
		return(number(chr));
	}

	/* three character operator ? */
	for (op = &Optab[0]; op->term; op++)
		if (sequal(op->term, buf))
			break;
	if (!op->term)
	{
		/* two character operator ? */
		backup(buf[2]);
		buf[2] = '\0';
		for (op = &Optab[0]; op->term; op++)
			if (sequal(op->term, buf))
				break;
		if (!op->term)
		{
			backup(buf[1]);
			buf[1] = '\0';
			for (op = &Optab[0]; op->term; op++)
				if (sequal(op->term, buf))
					break;
			if (!op->term)
			{
				Lcase = save;
				/* invalid operator */
				par_error (BADOP, WARN, 0);
			}
		}
	}
	Lcase = save;
	if(op->token == Tokens.bgncmnt)
		return(comment());
	if(op->token == Tokens.sconst)
		return (string(op));
	Lastok.tok = op->term;
	Lastok.toktyp = Tokens.sconst;
	yylval = op->opcode;
	return (op->token);
}
