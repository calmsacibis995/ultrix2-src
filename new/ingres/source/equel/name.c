#ifndef lint
static	char	*sccsid = "@(#)name.c	1.1	(ULTRIX)	1/8/85";
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
**  NAME -- Process an identifier or keyword token.
**
**	Name gets the identifier that follows in the std.
**	input, and checks if it is a keyword.
**	An identifier is defined as a sequence of
**	MAXNAME or fewer alphanumerics, starting with an
**	alphabetic character.
**
**	Parameters:
**		chr - the first character of the identifier
**
**	Returns:
**		Tokens.sp_name - for a user-defined name
**		Tokens.sp_struct_var -- if the name is declared 
**			a structurw variable
**		other - lexical codes for keys
**
**	Side Effects:
**		Adds a token to the symbol space.
**		yylval is set to the new node in the space.
**		If the identifier is a keyword, sets Opcode to
**		op_code from tokens.y.
*/

name(chr)
char		chr;
{
	int			lval;
	register		i;
	char			wbuf [MAXNAME + 1];
	register char		*cp;
	register char		c;
	struct optab		*op;
	struct optab		*getkey();
	struct cvar		*getcvar();

	c = chr;
	cp = wbuf;
	for (i = 0; i <= MAXNAME; i++)
	{
		lval = Cmap [c];
		if (i < MAXNAME &&
		   (lval == ALPHA || lval == NUMBR))
		{
			*cp++ = c;
			c = getch();
		}
		else if (lval == ALPHA || lval == NUMBR)
		{
			/* {i == MAXNAME && "c is legal" && 
			 *  cp == &wbuf [MAXNAME]} 
			 */
			*cp = '\0';
			yysemerr("name too long", wbuf);
			/* chomp to end of identifier */

			do
			{
				c = getch();
				lval = Cmap [c];
			}  while (lval == ALPHA || lval == NUMBR);
			backup(c);
			
			/* take first MAXNAME characters as IDENTIFIER 
			 * (non-key)
			 */
			yylval.u_dn = addsym(salloc(wbuf));
			return (Tokens.sp_name);
		}
		else
		{
			/* {cp <= &wbuf [MAXNAME] && i <= MAXNAME
			 * && "c is not part of id"}
			 */
			backup(c);
			*cp = '\0';
			i = 0;
			break;
		}
	}
	op = getkey(wbuf);

	/* Is it a keyword ? */
	if (op)
	{
		yylval.u_dn = addsym(op->op_term);
		Opcode = op->op_code;
		return (op->op_token);
	}
	/* user-defined name */
	yylval.u_dn = addsym(salloc(wbuf));
	if (getcvar(wbuf)->c_type == opSTRUCT)
		return(Tokens.sp_struct_var);
	return (Tokens.sp_name);
}
