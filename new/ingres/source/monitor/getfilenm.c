#ifndef lint
static	char	*sccsid = "@(#)getfilenm.c	1.1	(ULTRIX)	1/8/85";
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
**  GET FILE NAME
**
**	This routine collects a file name up to a newline and returns a
**	pointer to it.  Keep in mind that it is stored in a static
**	buffer.
**
**	Trace Flags:
**		40
*/

char *
getfilenm()
{
	static char	filename[81];
	register char	c;
	register int	i;
	register char	*p;
	extern char	getch();

	Oneline = TRUE;
	macinit(getch, 0, 0);

	/* skip initial spaces */
	while ((c = macgetch()) == ' ' || c == '\t')
		continue;

	i = 0;
	for (p = filename; c > 0; )
	{
		if (i++ <= 80)
			*p++ = c;
		c = macgetch();
	}
	*p = '\0';
	Prompt = Newline = TRUE;

#	ifdef xMTR2
	if (tTf(40, 0))
		printf("filename \"%s\"\n", filename);
#	endif
	Oneline = FALSE;
	getc(Input);
	return (filename);
}
