#ifndef lint
static	char	*sccsid = "@(#)clrline.c	1.1	(ULTRIX)	1/8/85";
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
**  Clear Input Line
**
**	This routine removes the newline following a monitor command
**	(such as \t, \g,  etc.)  Any other characters are processed.
**	Hence, \t\g\t will work.  It also maintains the
**	Newline flag on command lines.  It will make certain that
**	the current line in the query buffer ends with a newline.
**
**	The flag 'noprompt' if will disable the prompt character if set.
**	Otherwise, it is automatically printed out.
**
**	Uses trace flag 8
*/

clrline(noprompt)
int	noprompt;
{
	register char	c;

	if (!Newline)
		putc('\n', Qryiop);
	Newline = TRUE;
	/* if char following is a newline, throw it away */
	c = getch();
	Prompt = c == '\n';
	if (!Prompt)
	{
		ungetc(c, Input);
	}
	else
	{
		if (!noprompt)
			prompt(0);
	}
	return;
}
