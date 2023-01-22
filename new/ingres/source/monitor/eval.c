#ifndef lint
static	char	*sccsid = "@(#)eval.c	1.1	(ULTRIX)	1/8/85";
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

# include	"monitor.h"
# include	<ingres.h>
# include	<aux.h>




/*
**  DO MACRO EVALUATION OF QUERY BUFFER
**
**	The logical query buffer is read and passed through the macro
**	processor.  The main purpose of this is to evaluate {define}'s.
**	If the 'pr' flag is set, the result is printed on the terminal,
**	and so becomes a post-evaluation version of print.
**
**	Uses trace flag 12
*/

eval(pr)
int	pr;
{
	register FILE	*tfile;
	register char	c;
	extern int	fgetc();
	char		tfilename[40];

	Autoclear = 0;
	clrline(1);

	/* open temp file and reopen query buffer for reading */
	if (!pr)
	{
		concat("/tmp/INGTQ", Fileset, tfilename);
		if ((tfile = fopen(tfilename, "w")) == NULL)
			syserr("eval: open(%s)", tfilename);
	}
	if (freopen(Qbname, "r", Qryiop) == NULL)
		syserr("eval: freopen 1");

	/* COPY FILE */
	macinit(fgetc, Qryiop, 1);
	while ((c = macgetch()) > 0)
	{
		if (pr)
			putchar(c);
		else
			if (putc(c, tfile) == EOF)
				syserr("eval: putc");
	}

	if (!pr)
	{
		/* link temp file back to query buffer */
		fclose(tfile);
		unlink(Qbname);
		if (link(tfilename, Qbname))
			syserr("eval: link");
		unlink(tfilename);
	}

	/* reopen query buffer (now evaluated) */
	if (freopen(Qbname, "a", Qryiop) == NULL)
		syserr("eval: freopen 2");

	cgprompt();
}
