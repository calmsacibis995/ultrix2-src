#ifndef lint
static	char	*sccsid = "@(#)s_string.c	1.1	(ULTRIX)	1/8/85";
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
** STRING
** A string is defined as any sequence of MAXSTRING or fewer characters,
** surrounded by string delimiters.  New-line ;characters are purged
** from strings unless preceeded by a '\'; QUOTE's must be similarly
** prefixed in order to be correctly inserted within a string.  Each
** string is entered in the symbol table, indexed by 'yylval'.  A
** token or the error condition -1 is returned.
*/
string(op)
struct optab	*op;
{
	extern char	Cmap[];
	extern char	*yylval;
	extern char	*syment();
	register int	esc;
	register int	save;
	register char	*ptr;
	char		buf[MAXSTRING + 1];

	/* disable case conversion and fill in string */
	ptr = buf;
	save = Lcase;
	Lcase = 0;
	do
	{
		/* get next character */
		if ((*ptr = get_scan(NORMAL)) <= 0)
		{
			Lcase = save;
			/* non term string */
			par_error(STRTERM, FATAL, 0);
		}

		/* handle escape characters */
		esc = (*ptr == '\\');
		if (*ptr == '\n')
		{
			if ((*ptr = get_scan(NORMAL)) <= 0)
			{
				Lcase = save;
				*ptr = 0;
				/* non term string */
				par_error(STRTERM, FATAL, 0);
			}
		}
		if (esc == 1)
		{
			if ((*++ptr = get_scan(NORMAL)) <= 0)
			{
				Lcase = save;
				*ptr = 0;
				/* non term string */
				par_error(STRTERM, FATAL, 0);
			}
			if (*ptr == *(op->term))
				*--ptr = *(op->term);
		}

		/* check length */
		if ((ptr - buf) > MAXSTRING - 1)
		{
			Lcase = save;
			/* string too long */
			par_error(STRLONG, WARN, 0);
		}
		if (Cmap[*ptr] == CNTRL)
			/* cntrl in string from equel */
			par_error(CNTRLCHR, WARN, 0);
	} while (*ptr++ != *(op->term) || esc == 1);

	/* restore case conversion and return */
	*--ptr = '\0';
	Lcase = save;
#	ifdef	xSTR2
	tTfp(71, 8, "STRING: %s\n", buf);
#	endif
	yylval = syment(buf, (ptr - buf) + 1);
	Lastok.tok = yylval;
	Lastok.toktyp = Tokens.sconst;
	return (Tokens.sconst);
}
