#ifndef lint
static	char	*sccsid = "@(#)yyerror.c	1.1	(ULTRIX)	1/8/85";
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


extern	int	Exit_val;		/* value to exit with, incremented if error found */


/*
**  YYERROR -- Yacc error reporting routine.
**	Yyerror reports on syntax errors encountered by 
**	the yacc parser, and increments the Exit_val variable. 
**
**	Parameters:
**		s -- a string explaining the error
**
**	Returns:
**		none
*/


yyerror(s)
char	*s;
{

	printf("\"%s\", line %d: ", Input_file_name, yyline);
	if (yychar == 0)
		printf("EOF = ");
	if (yylval.u_dn)
		printf("\"%s\" ", yylval.u_dn->d_elm);
	printf("%s\n", s);
	Exit_val++;
}
/*
**  YYSEMERR -- scanner error reporter
**		Also increments the Exit_val variable.
**	Parameters:
**		s -- string explaining the error
**		i -- if !0 a string which caused the error
**
**	Returns:
**		none
**
**	Called By:
**		lexical analysis routines -- if called from somewhere else,
**			the line number is likely to be wrong.
*/


yysemerr(s, i)
char		*s;
char		*i;
{
	char	*str;

	printf("\"%s\", line %d: ", Input_file_name, yyline);
	if (i)
		printf("\"%s\": ", i);
	printf("%s\n", s);
	Exit_val++;
}
/*
**  YYSERROR -- Semantic error reportin routine
**	reports on an error on an entry in the symbol space,
**	using the line number built into the entry. Exit_val gets
**	incremented.
**
**	Parameters:
**		s -- a string explaining the error
**		d -- a symbol space node
**
**	Returns:
**		none
**
**	Called By:
**		semantic productions
*/


yyserror(s, d)
char			*s;
struct disp_node	*d;
{
	printf("\"%s\", line %d: ", Input_file_name, d->d_line);
	printf("\"%s\": %s\n", d->d_elm, s);
	Exit_val++;
}
