#ifndef lint
static	char	*sccsid = "@(#)string.c	1.1	(ULTRIX)	1/8/85";
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

# include	<stdio.h>
# include	"constants.h"
# include	"globals.h"
# include	"y.tab.h"



/*
**  STRING -- processes a string constant
**	Strings are kept internally exactly as their external
**	appearance, except for the outermost '"'.
**	A string may be at most MAXSTRING characters
**	long, and may have escaped newlines.
**
**	Parameters:
**		op -- pointer to string quote operator
**			table entry.
**	
**	Returns:
**		SCONST
*/


string(op)
struct optab	*op;
{
	char		buf [MAXSTRING + 1];
	register char	c, *cp;
	int		error;
	register int	escape;

	error = escape = 0;
	cp = buf;
	for ( ; ; )
	{
		c = getch();
		switch (c)
		{
		  
		  case '\\' :
			if (!escape)
				escape = 2;
			goto regchar;

		  case '\n' :
			if (escape)
				goto regchar;
			*cp = '\0';
			yysemerr("non-terminated string", 
			  !error ? buf : 0);
			break;

		  case EOF_TOK : 
			backup(c);
			*cp = '\0';
			yysemerr("EOF in string",
			  !error ? buf : 0);
			break;

		  default :
regchar :
			if (c == *op->op_term && !escape)
			{
				/* end of string */
				*cp = '\0';
				break;
			}
			if (!error)
			{
				if (cp - buf < MAXSTRING)
				{
					if (Cmap [c] == CNTRL)
						yysemerr("control character in string eliminated",
						0);
					else
						*cp++ = c;
				}
				else
				{
					yysemerr("string too long, rest discarded",
					0);
					error = 1;
				}
			}
			if (escape)
				--escape;
			continue;
		}
		break;
	}
	yylval.u_dn = addsym(salloc(buf));
	return (Tokens.sp_sconst);
}
