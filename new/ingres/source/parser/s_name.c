#ifndef lint
static	char	*sccsid = "@(#)s_name.c	1.1	(ULTRIX)	1/8/85";
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
** NAME
** A name is defined to be a sequence of MAXNAME or fewer alphanumeric
** characters, starting with an alphabetic (underscore "_" is considered
** an alphabetic).  If it is not a keyword, each name is entered into
** the symbol table, indexed by 'yylval'.  A token is then returned for
** that name.
*/
name(chr)
char	chr;
{
	extern char		*yylval;
	extern char		Cmap[];
	char			namebuf[MAXNAME + 1];
	register int		hi, lo, curr;
	extern char		*syment();

	/* fill in the name */
	yylval = namebuf;
	*yylval = chr;
	do
	{
		*++yylval = get_scan(NORMAL);
		if ((yylval - namebuf) > MAXNAME)
		{
			/* name too long */
			*yylval = '\0';
			par_error(NAMELONG, WARN, namebuf, 0);
		}

	}  while (Cmap[*yylval] == ALPHA || Cmap[*yylval] == NUMBR);
	backup(*yylval);
	*yylval = '\0';

	/* is it a keyword ? */
	lo = 0;
	hi = Keyent - 1;
	while (lo <= hi)
	{
		curr = (lo + hi) / 2;
		switch (scompare(Keyword[curr].term, MAXNAME, namebuf, MAXNAME))
		{
		  case 1:
			hi = curr - 1;
			continue;

		  case -1:
			lo = curr + 1;
			continue;

		  case 0:
			Lastok.toktyp = Tokens.sconst;
			Lastok.tok = Keyword[curr].term;
			Lastok.tokop = Keyword[curr].opcode;
			yylval = (char *) Lastok.tokop;
			return (Keyword[curr].token);
		}
	}

	/* else, USER DEFINED NAME */
#	ifdef	xSTR2
	tTfp(71, 0, "name: %s\n", namebuf);
#	endif
	yylval = syment(namebuf, length(namebuf) + 1);
	Lastok.tok = yylval;
	Lastok.toktyp = Tokens.sconst;
	Lastok.tokop = 0;
	return (Tokens.name);
}
