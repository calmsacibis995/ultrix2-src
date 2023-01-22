#ifndef lint
static	char	*sccsid = "@(#)yylex.c	1.1	(ULTRIX)	1/8/85";
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
**  YYLEX -- Lexical analyzer
**	Yylex controls the return to the parser of lexemes,
**	and the copying out of C_CODE on the all to yylex 
**	after yylex() returned the C_CODE.
**
**	Returns:
**		Lexical tokens.
**
**	Side Effects:
**		Copies C code out on call after seeing it.
**		Puts terminals in symbol space.
**
**	Called By:
**		Yacc internals (yyparse()).
*/



yylex()
{
	register int		rtval;
	register char		chr;



	/* get next token */
	rtval = CONTINUE;

	while (rtval == CONTINUE)
	{
		if (C_code_flg)
		{
			copy_c_code();
			Newline = 1;
		}
		Pre_proc_flg = 0;
	
		/* END OF FILE ? */
		if ((chr = getch()) == EOF_TOK)
		{
#			ifdef xDEBUG
			if (Lex_debug)
				printf("end of file\n");
#			endif
			return (0);
		}
	
		/* Test for a line of C code */
		if (Newline && if_c_code(chr))
		{
			if (C_code_flg)
				continue;
			rtval = Tokens.sp_c_code;
			C_code_flg = 1;
			break;
		}
		else
		{
			C_code_flg = 0;
			if (Newline)
			{
				Newline = 0;
				continue;
			}
		}
	
		/* CARRIAGE CONTROL ? */
		Newline = chr == '\n';

		switch (Cmap [chr])
		{

		  case PUNCT :
			continue;

		  case OPATR :
			rtval = operator(chr);
			break;

		  case NUMBR :
			rtval = number(chr);
			break;

		  case ALPHA :
			rtval = name(chr);
			break;

		}
	}
	if (Lex_debug)
		printf("YYLEX : '%s'\n", 
		yylval.u_dn ? yylval.u_dn->d_elm : "");
	return (rtval);
}
/*
**  COPY_C_CODE -- Copies out a line of C code
**	The test for Charcnt != 0 is beacuse if a C pre-processor
**	line follows an equel line, and equate_lines() puts out no
**	newline, the initial '#' will not be on the beginning of 
**	the line. As is, if this should happen, then the line
**	with the '#' will be one line in the output ahead of 
**	where it should be.
*/


copy_c_code()
{
	char	ch [2];


	ch [1] = 0;
	equate_lines();
	if (Pre_proc_flg)
	{
		if (Charcnt != 0)
			w_raw("\n");
		w_raw("#");
	}
	do
	{
		if ((*ch = getch()) == EOF_TOK)
			return;
		w_raw(ch);
	} while (*ch != '\n');
}
/*
**  IF_C_CODE -- Test to see if a line is C code
**
**	Sees if a line begins with "##" to see if it is equel.
**
**	Parameters:
**		chr -- first char of line
**
**	Returns:
**		0 -- Quel line
**		1 -- C line
**	
**	Called By:
**		yylex()
*/


if_c_code(chr)
char	chr;
{
	for ( ; ; )
	{
		if (chr != '#')
		{
			backup(chr);
			return (1);
		}
		Pre_proc_flg = 1;
		if ((chr = getch()) == EOF_TOK)
		{
			return (0);
		}
		if (chr != '#')
		{
			backup(chr);
			return (1);
		}
		else
		{
			return (0);
		}
	}
}
